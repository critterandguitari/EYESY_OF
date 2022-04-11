/*
 * Copyright (c) 2020 Owen Osborn, Critter & Gutiari, Inc.
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */
#include "ofApp.h"
#include <sys/types.h>
#include <ifaddrs.h>

//--------------------------------------------------------------
string parseIP() {
	string str = ofSystem( "ifconfig" );
	int place = str.find("wlan0",0);
	string str2 = str.substr(place+5, 200);
	int place2 = str2.find("inet", 0);
	return str2.substr(place2+5,13);
}

//--------------------------------------------------------------
std::string getWifiName() {
	return ofSystem( "iwgetid -r");
}

//--------------------------------------------------------------
string latestPNGs() {
	return ofSystem( "ls -Art | tail -n 4" );
	
}


//--------------------------------------------------------------
void ofApp::setup() {
    // declare the OS version
    osVersion = "oFLua 1.0"; 
    // listen on the given port
    cout << "listening for osc messages on port " << PORT << "\n";
    receiver.setup(PORT);  
    sender.setup("localhost", PORT+1);  
    
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    ofSetLogLevel("ofxLua", OF_LOG_VERBOSE);

    ofHideCursor();

    ofSetBackgroundColor(0,0,0);
        
    // setup audio
    soundStream.printDeviceList();
    
    int bufferSize = 256;
   

    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);
    
    bufferCounter    = 0;

    ofSoundStreamSettings settings;
    
    // device by name
    auto devices = soundStream.getMatchingDevices("default");
    if(!devices.empty()){
        settings.setInDevice(devices[0]);
    }

    settings.setInListener(this);
    //settings.sampleRate = 22050;
    settings.sampleRate = 11025;
    //settings.sampleRate = 44100;
    settings.numOutputChannels = 0;
    settings.numInputChannels = 2;
    settings.bufferSize = bufferSize;
    soundStream.setup(settings);    

    //some path, may be absolute or relative to bin/data
    string path = "/sdcard/Modes/oFLua"; 
    ofDirectory dir(path);
    dir.listDir();

    //go through and print out all the paths
    int countPaths = static_cast<int>(dir.size());
    for(int i = 0; i < countPaths; i++){
        ofLogNotice(dir.getPath(i) + "/main.lua");
        scripts.push_back(dir.getPath(i) + "/main.lua");
    }
        
    // scripts to run
    currentScript = 0;
    
    // init the lua state
    lua.init(true); // true because we want to stop on an error
    
    // listen to error events
    lua.addListener(this);
    
    // run a script
    // true = change working directory to the script's parent dir
    // so lua will find scripts with relative paths via require
    // note: changing dir does *not* affect the OF data path
    lua.doScript(scripts[currentScript], true);
    
    // call the script's setup() function
    lua.scriptSetup();

    // clear main screen
    ofClear(0,0,0);
    
    globalScene = 0;
    
    

    // osd setup
    osdW = ofGetScreenWidth();
    osdH = ofGetScreenHeight();
    osdEnabled = false;
    osdFont.load("CGFont_0.18.otf", osdH/45, true, true, true, 10, 64);
    osdFont.setLetterSpacing(1);
    osdFontK.load("CGFont_0.18.otf", osdH/68, true, true, true, 10, 64);
    osdFontK.setLetterSpacing(1);

    osdFbo.allocate(osdW*0.8, osdH);
    dummyAudio = 0;
    updateScreenGrabs();
    sizeScripts = scripts.size();
    
        
}

//--------------------------------------------------------------
void ofApp::update() {
    globalTrig = false;
    
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
	// get various key messages
        if(m.getAddress() == "/key") {   
            
            
            if (m.getArgAsInt32(0) == 9 && m.getArgAsInt32(1) > 0) {
                img.grabScreen(0,0,ofGetWidth(),ofGetHeight());
                string fileName = "snapshot_"+ofToString(10000+snapCounter)+".png";
                cout << "saving " + fileName + "...";
                img.save("/sdcard/Grabs/" + fileName);
                cout << "saved\n";
                updateScreenGrabs();
		
            }
	    if (m.getArgAsInt32(0) == 10 && m.getArgAsInt32(1) == 0) dummyAudio = 0;
            if (m.getArgAsInt32(0) == 10 && m.getArgAsInt32(1) > 0) {
		dummyAudio = 1;
                cout << "trig" << "\n";
                globalTrig = true;
		
            }
       	    if (m.getArgAsInt32(0) == 1) {
		osdEnabled = ( m.getArgAsInt32(1) > 0 ) ? true : false; 
		cout << "change OSD: " << osdEnabled << "\n";
            } 
	}
	if(m.getArgAsInt32(0) == 15) {
		shIft = ( m.getArgAsInt32(1) > 0 ) ? true : false;
	}

	if(m.getAddress() == "/seq") {
		seqStatus = m.getArgAsInt32(0); 
	}
	
	// scene recall
	if(m.getAddress() == "/sceneRecall") {
		globalScene = m.getArgAsInt32(0); 
		recallScript( m.getArgAsInt32(1) );
		sendCurrentScript( currentScript );		
	}
	if (m.getAddress() == "/modeDown" && m.getArgAsInt32(0) > 0) {
                cout << "back" << "\n";
                prevScript();
		sendCurrentScript( currentScript );
        }
        if (m.getAddress() == "/modeUp" && m.getArgAsInt32(0) > 0) {
                cout << "fwd" << "\n";
                nextScript();
		sendCurrentScript( currentScript );
	}
	
	// knobs
        if(m.getAddress() == "/knob1") {
		k1Local = (float)m.getArgAsInt32(0) / 1023;
 		lua.setNumber("knob1", k1Local);
		if(m.getArgAsInt32(1) > 0) {
			k1Red = true;
		} else {
			k1Red = false;
		}
	}
	if(m.getAddress() == "/knob2") {
		k2Local = (float)m.getArgAsInt32(0) / 1023;
		lua.setNumber("knob2", k2Local);
		if(m.getArgAsInt32(1) > 0) {
			k2Red = true;
		} else {
			k2Red = false;
		}
	}
	if(m.getAddress() == "/knob3") {
		k3Local = (float)m.getArgAsInt32(0) / 1023;
		lua.setNumber("knob3", k3Local);
		if(m.getArgAsInt32(1) > 0) {
			k3Red = true;
		} else {
			k3Red = false;
		}
	}
	if(m.getAddress() == "/knob4") {
		k4Local = (float)m.getArgAsInt32(0) / 1023;
		lua.setNumber("knob4", k4Local);
		if(m.getArgAsInt32(1) > 0) {
			k4Red = true;
		} else {
			k4Red = false;
		}
	}	
	if(m.getAddress() == "/knob5") {
		k5Local = (float)m.getArgAsInt32(0) / 1023;
		lua.setNumber("knob5", k5Local);
		if(m.getArgAsInt32(1) > 0) {
			k5Red = true;
		} else {
			k5Red = false;
		}
	}

	 
	if(m.getAddress() == "/updateSceneCount") {
		totalScenes = m.getArgAsInt32(0);
	}
	// midi
	if(m.getAddress() == "/midinote") {
		osdMidi[0] = m.getArgAsInt32(0);
		osdMidi[1] = m.getArgAsInt32(1); 	
		lua.setNumber("midiNote", osdMidi[0] );
		lua.setNumber("midiVel", osdMidi[1] );	
		midiTable[osdMidi[0]] = osdMidi[1];
	}
	if(m.getAddress() == "/midiclock") {
		lua.setBool("midiClock", true);
	}
	
 	
	if(m.getAddress() == "/printTrig") {
		float wow = ((float)m.getArgAsInt32(0)/1023) * 5; 
		globalTrigInput = floor( wow + 0.49999);
	}

	// trigger
	if(m.getAddress() == "/trig") {
		if( m.getArgAsInt32(0) > 0 && globalTrigInput > 0) {
			globalTrig = true;
		} 
	}
	// detect link
	if(m.getAddress() == "/linkpresent" ) {
		if(m.getArgAsInt32(0) > 0) {
			globalLink = true;
		} else {
			globalLink = false;
		}
	} 

	if(m.getAddress() == "/gain") {
		globalGain = ((float)m.getArgAsInt32(0) / 1023) * 3;
	}	
	if(m.getAddress() == "/midiChannel") {
		float wow = ((float)m.getArgAsInt32(0)/1023) * 15;
		globalMidiChannel = floor(wow + 0.49999) + 1;
	}	
	if(m.getAddress() == "/reload") {
            	cout << "reloading\n";
            	reloadScript();
        }
    }
    // calculate peak for audio in display
    float peAk = 0;
    for (int i = 0; i < 256; i++) {
	float peakAbs = abs( left[i] );
	    if (left[i] > peAk ) {
	    	peAk = peakAbs;
	    }
    } 
    // audio trig if selected
    if(globalTrigInput == 0) {
    	if( peAk >= 0.75 ){
		globalTrig = true;
	}
    }	
    // trigger speaks to lua
    if( globalTrig == true) {
    	lua.setBool("trig", true);
    } else {
	lua.setBool("trig", false);
    }

    // call the script's update() function
    lua.scriptUpdate();
    
    //// OSD fill the fbo 
    if (osdEnabled == true) {
		
	float spaceTrack = 0;
	float fontHeight = floor( osdFont.stringHeight( "Lpl" ) + 4) ;
	
	// begin the fbo
	osdFbo.begin();
		ofClear(255,255,255,0);
		
		// mode name
		ofPushMatrix();
			ofTranslate(0,0);
			std::stringstream scrpz;
			scrpz << "Mode: " << lua.getString("modeTitle");
    			float scrpW = osdFont.stringWidth( scrpz.str() );
			ofSetColor(0);
			ofFill();
			
			ofDrawRectangle(0,0,scrpW+4,fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString(scrpz.str(), 2, fontHeight-4 );
		ofPopMatrix();

		// Explain the Mode 
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			std::stringstream eXplain;
			eXplain << "Mode Description: " << lua.getString("modeExplain");
			float eXwith = osdFont.stringWidth( eXplain.str() );
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,eXwith+4,fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString( eXplain.str(), 2, fontHeight-4);

		ofPopMatrix();
		
		// knobs
		ofPushMatrix();
			// draw background
			float knobW = floor(osdW/38);
			float knobH = ceil(osdH/5.4);
			float knobTextH = floor(osdH/68);
			ofFill();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			ofSetColor(0);
			ofDrawRectangle(0,0,knobW*16,knobH+(knobTextH*3));
			spaceTrack += knobH+(knobTextH*3);
			
			// draw k1
			ofPushMatrix();
				ofTranslate(knobW/2, knobW/2);
				if (k1Red==false) {ofSetColor(255);} else {ofSetColor(255,0,0);}
				ofDrawRectangle(0,0,knobW,knobH);
				ofSetColor(255);	
				osdFontK.drawString( "Knob1",0,-(knobTextH/2));
				std::stringstream k1Name;
				k1Name << lua.getString("titleK1");
				osdFontK.drawString( k1Name.str(), 0, knobH+(knobTextH) );
				ofSetColor(0);
				ofDrawRectangle(1,1,knobW-2,floor((1-k1Local)*(knobH-2)) );
			ofPopMatrix();
			// draw k2
			ofPushMatrix();
				ofTranslate(knobW*3+(knobW/2), knobW/2);
				if (k2Red==false) {ofSetColor(255);} else {ofSetColor(255,0,0);}
				ofDrawRectangle(0,0,knobW,knobH);
				ofSetColor(255);
				osdFontK.drawString( "Knob2",0,-(knobTextH/2) );
				std::stringstream k2Name;
				k2Name << lua.getString("titleK2");
				osdFontK.drawString( k2Name.str(), 0, knobH + knobTextH);
				ofSetColor(0);
				ofDrawRectangle(1,1,knobW-2,floor((1-k2Local)*(knobH-2)) );
			ofPopMatrix();
			// draw k3
			ofPushMatrix();
				ofTranslate(knobW*6+(knobW/2), knobW/2);
				if (k3Red==false) {ofSetColor(255);} else {ofSetColor(255,0,0);}	
				ofDrawRectangle(0,0,knobW,knobH);
				ofSetColor(255);
				osdFontK.drawString( "Knob3",0,-(knobTextH/2));
				std::stringstream k3Name;
				k3Name << lua.getString("titleK3");
				osdFontK.drawString( k3Name.str(), 0, knobH + knobTextH);
				ofSetColor(0);
				ofDrawRectangle(1,1,knobW-2,floor((1-k3Local)*(knobH-2)) );
			ofPopMatrix();
			// draw k4
			ofPushMatrix();
				ofTranslate(knobW*9+(knobW/2), knobW/2);
				if (k4Red==false) {ofSetColor(255);} else {ofSetColor(255,0,0);}	
				ofDrawRectangle(0,0,knobW,knobH);
				ofSetColor(255);
				osdFontK.drawString( "Knob4",0,-(knobTextH/2));
				std::stringstream k4Name;
				k4Name << lua.getString("titleK4");
				osdFontK.drawString( k4Name.str(), 0, knobH+knobTextH);
				ofSetColor(0);
				ofDrawRectangle(1,1,knobW-2,floor((1-k4Local)*(knobH-2)) );
			ofPopMatrix();
			// draw k5
			ofPushMatrix();
				ofTranslate(knobW*12+(knobW/2), knobW/2);
				if (k5Red==false) {ofSetColor(255);} else {ofSetColor(255,0,0);}	
				ofDrawRectangle(0,0,knobW,knobH);
				ofSetColor(255);
				osdFontK.drawString( "Knob5",0,-(knobTextH/2) );
				std::stringstream k5Name;
				k5Name << lua.getString("titleK5");
				osdFontK.drawString( k5Name.str(), 0, knobH+knobTextH);
				ofSetColor(0);
				ofDrawRectangle(1,1,knobW-2,floor((1-k5Local)*(knobH-2)) );
			ofPopMatrix();
		ofPopMatrix();

		// volume
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			std::stringstream inputStr;
			inputStr <<  "Input Level: ";
			float volChunk = floor(osdW/160);
			float visVol = peAk * 16.0;
			float volStrWidth = osdFont.stringWidth( inputStr.str() );
			
			ofSetColor(0);
			ofDrawRectangle(0,0,volStrWidth+(volChunk*20), volChunk*6 );
			spaceTrack += volChunk*6;
			ofSetColor( 255 );
			osdFont.drawString( inputStr.str(), 2, fontHeight+2);
			// draw the rectangles
			for ( int i=0; i<16; i++) {
			       	float xPos = (i*volChunk)+volChunk;
				ofSetColor( 255 );
				if ((i+1) <= visVol ) {
					ofFill();
					if(i<10) {
						ofSetColor(0,255,0);
					} else if(i >= 10 and i < 13) {
						ofSetColor(255,255,0);
					} else {
						ofSetColor(255,0,0);
					}
					ofDrawRectangle(xPos+(volStrWidth), volChunk, volChunk,(volChunk*4) );
				}
				ofNoFill();
				ofSetColor(255);
				ofDrawRectangle(xPos+volStrWidth, volChunk, volChunk, volChunk*4 );
			}	
		ofPopMatrix();
		
		// Trigger
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2) );
			spaceTrack += fontHeight/2;
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,knobW*4,knobW+(volChunk*2));
			spaceTrack += knobW+(volChunk*2);
			ofSetColor(255);
			
 			osdFont.drawString( "Trigger: ", 2, fontHeight+2);
			
			if (globalTrig == true) {
				ofSetColor(255,255,0);
				ofFill();
				ofDrawRectangle( knobW*2, volChunk, knobW, knobW);
			}
			// draw white border
			ofNoFill();
			ofSetColor(255);
			ofDrawRectangle( knobW*2,volChunk , knobW, knobW);
			
			
					
		ofPopMatrix();	
		
		// midi
		ofPushMatrix();
			float chunk = ceil(osdH/108);
			ofTranslate(0, spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			
			std::stringstream midStr;
			midStr << "MIDI: ";
			float midiW = osdFont.stringWidth( midStr.str() );
			
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,(midiW+(chunk*20)),(chunk*9));
			spaceTrack += (chunk*9);
			ofSetColor(255);
			
			

			osdFont.drawString( midStr.str(), 2, fontHeight+2);
			osdFontK.drawString( "0", midiW, ceil(chunk*1.5) );
			osdFontK.drawString( "127", (midiW+(chunk*17))+2 ,(chunk/2)+(chunk*8) );
			for ( int i=0; i<9; i++) {
				// draw horizontal lines
				int yPos = (i*chunk) + ceil(chunk/2);
				ofDrawLine(midiW+chunk,yPos,ceil(chunk*23),yPos);
			}
			for (int i=0; i<17; i++) {
				// draw vertical lines
				int xPos = (i*chunk) + (midiW+chunk);
				ofDrawLine(xPos,ceil(chunk/2),xPos,ceil(chunk/2)+ceil(chunk*8));
			}
			
			for(int i=0; i<128; i++) {
				if (midiTable[i] != 0) {
					float xPos = ((i % 16) * chunk)+(midiW+chunk);
					float yPos = (floor( i / 16 ) * chunk) + ceil(chunk/2); 
					ofSetColor(0,255,255);
					ofFill();
					ofDrawRectangle(xPos,yPos, chunk, chunk);
				}
			}
		
			
		ofPopMatrix();
		
		// Sequencer 
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			std::stringstream seQ;
			if( seqStatus == 1) seQ << "Sequencer: " << "Ready To Record";
			if( seqStatus == 2) seQ << "Sequencer: " << "Recording";
			if( seqStatus == 3) seQ << "Sequencer: " << "Playing";
			if( seqStatus == 0) seQ << "Sequencer: " << "Stopped";

			
			float seqW = osdFont.stringWidth( seQ.str() );
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,seqW+4,fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString( seQ.str(), 2, fontHeight-4);
		ofPopMatrix();

		//WIFI
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			std::stringstream wifI;
			wifI << "WIFI: " << getWifiName();
			ofFill();
			ofSetColor(0);
			float wifiW = osdFont.stringWidth( wifI.str() );
			ofDrawRectangle(0,0,wifiW+4, fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString(wifI.str(), 2, fontHeight-4);
		ofPopMatrix();

		//IP
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			
			std::stringstream ipAd;
			ipAd << "IP Address: " << parseIP();
			ofFill();
			ofSetColor(0);
			float ipW = osdFont.stringWidth( ipAd.str() );
			ofDrawRectangle(0,0,ipW+4, fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString(ipAd.str(), 2, fontHeight-4);
		ofPopMatrix();

		//OS Version
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += fontHeight/2;
			std::stringstream osStr;
			osStr << "OS Version: " << osVersion;
			ofFill();
			ofSetColor(0);
			float osW = osdFont.stringWidth( osStr.str() );
			ofDrawRectangle(0,0,osW+4, fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString(osStr.str(), 2, fontHeight-4);
		ofPopMatrix();
			
		
		// FPS
		ofPushMatrix();
			ofTranslate(0,spaceTrack + (fontHeight/2));
			spaceTrack += (fontHeight/2);
			std::stringstream fPs;
			float getFramz = ofGetFrameRate();
			int getFramzI = static_cast<int>(getFramz);
			fPs << "FPS: " << getFramzI ;
			float fpsW = osdFont.stringWidth( fPs.str() );
			ofSetColor(0);
			ofDrawRectangle(0,0,fpsW+4,fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString(fPs.str(), 2, fontHeight-4);
		ofPopMatrix();	
		
		// Scene Stuff
		ofPushMatrix();
			ofTranslate(0, spaceTrack + (fontHeight/2));
			spaceTrack += (fontHeight/2);
			std::stringstream scenE;
			if(globalScene == 0) {
				scenE << "Scene: " << "None" << " of " << totalScenes;
			} else {

				scenE << "Scene: " << globalScene << " of " << totalScenes;
			}
			float sceneW = osdFont.stringWidth( scenE.str() );
			ofSetColor(0);
			ofDrawRectangle(0,0,sceneW+4, fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString(scenE.str(), 2, fontHeight-4);
		ofPopMatrix();
		
		// Screen Grabs
		ofPushMatrix();
			ofTranslate(0, spaceTrack + (fontHeight/2));
			spaceTrack += (fontHeight/2);
			std::stringstream graBz;
			graBz << "Total Screen Grabs: " << snapCounter;	
			float grabW = osdFont.stringWidth( graBz.str() );
			ofSetColor(0);
			ofDrawRectangle(0,0,grabW+4, fontHeight);
			spaceTrack += fontHeight;
			ofSetColor(255);
			osdFont.drawString(graBz.str(), 2, fontHeight-4);
		ofPopMatrix();

		// draw the last 5 screen grabs
		ofPushMatrix();
			float grabHeit = ceil(osdH/10);
			ofTranslate( ceil(osdW/2), ceil(osdH/3) );
			grab1.resize( ceil(osdW/10), grabHeit);
			grab1.draw(0,0);
			ofTranslate( 0, grabHeit + (grabHeit/3) );
			grab2.resize( ceil(osdW/10), ceil(osdH/10) );
			grab2.draw(0,0);
			ofTranslate( 0, grabHeit + (grabHeit/3) );
			grab3.resize( ceil(osdW/10), ceil(osdH/10) );
			grab3.draw(0,0);
			ofTranslate( 0, grabHeit + (grabHeit/3) );
			grab4.resize( ceil(osdW/10), ceil(osdH/10) );
			grab4.draw(0,0);	
			
		ofPopMatrix();
			
		
		// draw the shift options if osd and shift is on
		if (shIft == true) {
			ofPushMatrix();
				// gain knob1
				ofTranslate(osdW/2,0);
				std::stringstream gAin;
				gAin << "Gain: " << ceil(globalGain*100) << "%";
    				float gW = osdFont.stringWidth( gAin.str() );
				ofSetColor(0);
				ofFill();
				ofDrawRectangle(0,0,gW+4,fontHeight);
				ofSetColor(255);
				osdFont.drawString(gAin.str(), 2, fontHeight-4);
			
			
				// trigger input
				ofTranslate(0,fontHeight*2);
				
				std::stringstream trigPut;
				if (globalTrigInput == 0) trigPut << "Trigger Input: Audio";
				if (globalTrigInput == 1) trigPut << "Trigger Input: Midi Clock Quarter Note";
				if (globalTrigInput == 2) trigPut << "Trigger Input: Midi Clock Eigth Note";
				if (globalTrigInput == 3) trigPut << "Trigger Input: Midi Notes";
				if (globalTrigInput == 4) trigPut << "Trigger Input: LINK Quarter Note";
				if (globalTrigInput == 5) trigPut << "Trigger Input: LINK Eigth Note";
				float trigW = osdFont.stringWidth( trigPut.str() );
				ofSetColor(0);
				ofFill();
				ofDrawRectangle(0,0,trigW+4,fontHeight);
				
				ofSetColor(255);
				osdFont.drawString( trigPut.str(),2,fontHeight-4);

				// Midi channel select
				ofTranslate(0,fontHeight*2);
				
				std::stringstream mIdChan;
				mIdChan << "MIDI Channel: " << globalMidiChannel;
				float mIdW = osdFont.stringWidth( mIdChan.str() );
				ofSetColor(0);
				ofFill();
				ofDrawRectangle(0,0,mIdW+4,fontHeight);
				ofSetColor(255);
				osdFont.drawString( mIdChan.str(),2,fontHeight-4);

			ofPopMatrix();

		}

	// end the fbo
	osdFbo.end();
    } 
    
    
}

//--------------------------------------------------------------
void ofApp::draw() {
    // set the audio buffer
    lua.setNumberVector("inL", left);
    lua.setNumberVector("inR", right);
    
    // draw the lua mode	
    // enable depth
    ofEnableDepthTest();

    ofPushMatrix();
    	lua.scriptDraw();
    ofPopMatrix();
    
    // disable depth
    ofDisableDepthTest();

    if (osdEnabled) {
	// draw it
	float marg = ceil( osdW/12 );
    	ofSetColor(255);
    	ofTranslate( marg, ceil(marg/2), 10);
    	osdFbo.draw(0,0);
    }
    lua.setBool("trig", false);
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
   
    if (!dummyAudio){	
        for (size_t i = 0; i < input.getNumFrames(); i++){
            left[i]  = input[i*2] * globalGain;
            right[i] = input[i*2+1] * globalGain;
	    
        }
    } else {
        for (size_t i = 0; i < input.getNumFrames(); i++){
            left[i]  = sin((i*TWO_PI)/input.getNumFrames());
            right[i] = cos((i*TWO_PI)/input.getNumFrames());
        }
    }
    
    bufferCounter++;
    
}

//--------------------------------------------------------------
void ofApp::exit() {
    // call the script's exit() function
    lua.scriptExit();
    
    // clear the lua state
    lua.clear();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    /*
    switch(key) {
    
        case 'r':
            reloadScript();
            break;
    
        case OF_KEY_LEFT:
            prevScript();
            break;
            
        case OF_KEY_RIGHT:
            nextScript();
            break;
            
        case ' ':
            lua.doString("print(\"this is a lua string saying you hit the space bar!\")");
            cout << "fps: " << ofGetFrameRate() << "\n";    
            break;
    }
    
    lua.scriptKeyPressed(key);
    */
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    //lua.scriptMouseMoved(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    //lua.scriptMouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
   // lua.scriptMousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
//    lua.scriptMouseReleased(x, y, button);
}

//--------------------------------------------------------------
void ofApp::errorReceived(std::string& msg) {
    ofLogNotice() << "got a script error: " << msg;
}


//--------------------------------------------------------------
void ofApp::reloadScript() {
    // exit, reinit the lua state, and reload the current script
    lua.scriptExit();
    
    // init OF
    ofSetupScreen();
    ofSetupGraphicDefaults();
    ofSetBackgroundColor(0,0,0);

    // load new
    lua.init();
    lua.doScript(scripts[currentScript], true);
    lua.scriptSetup();
}

void ofApp::nextScript() {
    currentScript++;
    if(currentScript > sizeScripts-1) {
        currentScript = 0;
    }
    reloadScript();
}

void ofApp::prevScript() {
    if(currentScript == 0) {
        currentScript = sizeScripts-1;
    }
    else {
        currentScript--;
    }
    reloadScript();
}

void ofApp::recallScript(int num) {
	currentScript = num;
	reloadScript();
}

void ofApp::sendCurrentScript(int cur) {
	// compose the message
	
	mess.setAddress("/currentScript");
	mess.addIntArg( cur );
	sender.sendMessage(mess);
}

void ofApp::updateScreenGrabs() {
	string grabPath = "/sdcard/Grabs/";
    	ofDirectory grabDir(grabPath);
	
	// initiate snap counter
    	grabDir.listDir(); 			// list them
    	grabDir.sort();				// sort them by name
    	int grabAmt = grabDir.size();		// get total amount
	snapCounter = grabAmt;			// update the Counter

   	cout << grabDir.getPath(grabAmt-1) << "\n"; 
    	grab1.load( grabDir.getPath(grabAmt-1) );
	grab2.load( grabDir.getPath(grabAmt-2) );
	grab3.load( grabDir.getPath(grabAmt-3) );
	grab4.load( grabDir.getPath(grabAmt-4) );
	
}
	
