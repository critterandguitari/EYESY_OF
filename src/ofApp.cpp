/*
 * Copyright (c) 2020 Owen Osborn, Critter & Gutiari, Inc.
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
*/

#include "ofApp.h"
#include <stddef.h>
#include <stdint.h>

// variables
float peAk = 0;
float scaleOsd = 1;
bool grabImgChange = true;
int osdCounter = 0;
int grabCounter = 0;

uint64_t cpuMaxLoop = 3000;
int countPaths;
bool globalTrig = false;
string modeTitle;
string modeDescrip;
string theWifiName;
string theIP;
float compareCpuCount = 0.0;
string grabPath = "/sdcard/Grabs/";
ofDirectory grabDir( grabPath );
ofImage grab1;
ofImage grab2;
ofImage grab3;
ofImage grab4;

// update the screen grabs
void updateScreenGrabs() {
	// initiate grab counter
    	grabDir.listDir(); 			// list them
    	grabDir.sort();				// sort them by name
    	grabCounter = grabDir.size();		// get total amount
	
	// cout << grabCounter << "\n";
   	if( grabCounter >= 1 ) { cout << grabDir.getPath(grabCounter-1) << "\n";}
    	if( grabCounter >= 1 ) { grab1.load( grabDir.getPath(grabCounter-1) );}
	if( grabCounter >= 2 ) { grab2.load( grabDir.getPath(grabCounter-2) );}
	if( grabCounter >= 3 ) { grab3.load( grabDir.getPath(grabCounter-3) );}
	if( grabCounter >= 4 ) { grab4.load( grabDir.getPath(grabCounter-4) );}
	
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
    
    ofSetLogLevel("ofxLua", OF_LOG_VERBOSE);
    
    // hide the cursor
    ofHideCursor();

    // setup audio
    soundStream.printDeviceList();
    
    // set the buffer sixe to 256
    int bufferSize = 256;
   
    left.assign(bufferSize, 0.0);
    //right.assign(bufferSize, 0.0);
    
    //bufferCounter    = 0;

    ofSoundStreamSettings 	settings;
    
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
    soundStream.setup( settings );    

    //some path, may be absolute or relative to bin/data
    string path = "/sdcard/Modes/oFLua"; 
    ofDirectory dir( path );
    dir.listDir();
    dir.sort();

    // look for grab image directory, if none create
    if( !grabDir.exists() ) {
    	grabDir.create( true );
    }


    //go through and print out all the paths
    countPaths = static_cast<int>( dir.size() );
    for(int i = 0; i < countPaths; i++){
        // print the list of modes to the eyesy terminal
	ofLogNotice(dir.getPath(i) + "/main.lua");
	// fill a c++ array named scripts with the paths
        scripts.push_back(dir.getPath(i) + "/main.lua");
    }
        
    // scripts to run, start a beginning
    currentScript = 0;
    
    // init the lua state
    lua.init( true ); // true because we want to stop on an error
    
    // listen to error events
    lua.addListener(this);
    
    // run a script
    // true = change working directory to the script's parent dir
    // so lua will find scripts with relative paths via require
    // note: changing dir does *not* affect the OF data path
    lua.doScript( scripts[currentScript], true);
    
    // call the script's setup() function
    lua.scriptSetup();

    // clear main screen
    ofClear(0,0,0);
    
    // osd setup
    osdW = ofGetScreenWidth()/scaleOsd;
    osdH = ofGetScreenHeight()/scaleOsd;
    
    osdFont.load("CGFont_0.18.otf", ceil(osdH/8), true, false, false, 1, 12);
    osdFont.setLetterSpacing(1);
    osdFontK.load("CGFont_0.18.otf", ceil(osdH/12), true, false, false, 1, 12);
    osdFontK.setLetterSpacing(1);

    dummyAudio = 0;
    updateScreenGrabs();
    sizeScripts = scripts.size();

    modeTitle = lua.getString("modeTitle");
    modeDescrip = lua.getString("modeExplain");

    thread.startThread();

    persistFbo.allocate(ofGetWindowWidth(), ofGetWindowHeight());

}

//--------------------------------------------------------------
void ofApp::update() {
    
    float time = ofGetElapsedTimeMillis() % cpuMaxLoop;
    if( time < compareCpuCount) {
	thread.lock();
    		theIP = thread.ip;
		theWifiName = thread.wifi;
   	thread.unlock();
	//cout << "its the lOOP!" << "\n";
	
    }
    compareCpuCount = time;

    
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
	
	// get various key messages
        if(m.getAddress() == "/key") {   
            	// grab screen
            	if (m.getArgAsInt32(0) == 9 && m.getArgAsInt32(1) > 0) {
            	    	img.grabScreen(0,0,ofGetWidth(),ofGetHeight());
            	    	string fileName = "/screenGrab_" + ofToString( 10000+grabCounter ) + ".png";
            	    	cout << "saving " +  grabDir.getAbsolutePath() + fileName + "...";
            	    	img.save( grabDir.getAbsolutePath() + fileName );
            	    	cout << "saved\n";
                	updateScreenGrabs();
			
	    	}
	    	// trigger button
	   	if (m.getArgAsInt32(0) == 10 && m.getArgAsInt32(1) > 0) {
			dummyAudio = 1;
                	cout << "trig" << "\n";
                	globalTrig = true;
            	} else {
			dummyAudio = 0;
			globalTrig = false;
		}

		// persist toggle button
		if (m.getArgAsInt32(0) == 3 && m.getArgAsInt32(1) > 0) {
			persistEnabled = !persistEnabled;
			persistFirstRender = true;
			cout << "change persist: " << persistEnabled << "\n";
		}

		// osd toggle button
       	    	if (m.getArgAsInt32(0) == 1) {
			osdEnabled = ( m.getArgAsInt32(1) > 0 ) ? true : false; 
			cout << "change OSD: " << osdEnabled << "\n";
            	}	 
	    	// shift button
	    	if(m.getArgAsInt32(0) == 15) {
			shIft = ( m.getArgAsInt32(1) > 0 ) ? true : false;
	    	}
	}	

	//// dedicated named osc buttons
	
	// sequencer live status
	if(m.getAddress() == "/seq") {
		seqStatus = m.getArgAsInt32(0); 
	}
	
	// scene recall
	if(m.getAddress() == "/sceneRecallString") {
		recallScript( m.getArgAsString(0) );
		cout << "recalled string: " << m.getArgAsString(0) << "\n";
	}	
	if(m.getAddress() == "/sceneRecall") {
		globalScene = m.getArgAsInt32(0); 
		sendCurrentScript( scripts[currentScript] );
		modeTitle = lua.getString("modeTitle");
		modeDescrip = lua.getString("modeExplain");		
	}
	// mode change button down
	if (m.getAddress() == "/modeDown" && m.getArgAsInt32(0) > 0) {
                prevScript();
		cout << "back" << "\n";
               	// update the script(mode) number to the PD
		sendCurrentScript( scripts[currentScript] );
		modeTitle = lua.getString("modeTitle");
		modeDescrip = lua.getString("modeExplain");
        }
	// mode change button up
        if (m.getAddress() == "/modeUp" && m.getArgAsInt32(0) > 0) {
                nextScript();
		cout << "fwd" << "\n";
		// update the script(mode) number to the PD
		sendCurrentScript( scripts[currentScript] );
		modeTitle = lua.getString("modeTitle");
		modeDescrip = lua.getString("modeExplain");
	}
	
	////// the knobs
        if(m.getAddress() == "/knob1") {
		k1Local = (float)m.getArgAsInt32(0) / 1022.0;
 		lua.setNumber("knob1", k1Local);
		if(m.getArgAsInt32(1) > 0) {
			k1Red = true;
		} else {
			k1Red = false;
		}
	}
	if(m.getAddress() == "/knob2") {
		k2Local = (float)m.getArgAsInt32(0) / 1022.0;
		lua.setNumber("knob2", k2Local);
		if(m.getArgAsInt32(1) > 0) {
			k2Red = true;
		} else {
			k2Red = false;
		}
	}
	if(m.getAddress() == "/knob3") {
		k3Local = (float)m.getArgAsInt32(0) / 1022.0;
		lua.setNumber("knob3", k3Local);
		if(m.getArgAsInt32(1) > 0) {
			k3Red = true;
		} else {
			k3Red = false;
		}
	}
	if(m.getAddress() == "/knob4") {
		k4Local = (float)m.getArgAsInt32(0) / 1022.0;
		lua.setNumber("knob4", k4Local);
		if(m.getArgAsInt32(1) > 0) {
			k4Red = true;
		} else {
			k4Red = false;
		}
	}	
	if(m.getAddress() == "/knob5") {
		k5Local = (float)m.getArgAsInt32(0) / 1022.0;
		lua.setNumber("knob5", k5Local);
		if(m.getArgAsInt32(1) > 0) {
			k5Red = true;
		} else {
			k5Red = false;
		}
	}
	// total scene count 	 
	if(m.getAddress() == "/updateSceneCount") {
		totalScenes = m.getArgAsInt32(0);
	}
	// midi note inputs
	if(m.getAddress() == "/midinote") {
		osdMidi[0] = m.getArgAsInt32(0);
		osdMidi[1] = m.getArgAsInt32(1); 	
		lua.setNumber("midiNote", osdMidi[0] );
		lua.setNumber("midiVel", osdMidi[1] );	
		midiTable[osdMidi[0]] = osdMidi[1];
	}
	
	// trigger input modes
 	if(m.getAddress() == "/printTrig") {
		float wow = ((float)m.getArgAsInt32(0)/1023) * 5; 
		globalTrigInput = floor( wow + 0.49999);
	}
	
	// detect link
	if(m.getAddress() == "/linkpresent" ) {
		if(m.getArgAsInt32(0) > 0) {
			globalLink = true;
		} else {
			globalLink = false;
		}
	}

	// gain level from the pure data
	if(m.getAddress() == "/gain") {
		globalGain = ((float)m.getArgAsInt32(0) / 1023) * 3;
	}

	// midi channel from pure data
	if(m.getAddress() == "/midiChannel") {
		float wow = ((float)m.getArgAsInt32(0)/1023) * 15;
		globalMidiChannel = floor(wow + 0.49999) + 1;
	}

	// reload function
	if(m.getAddress() == "/reload") {
            	cout << "reloading\n";
            	reloadScript();
        }

    } // end of receiving messages from pure data

    // calculate peak for audio in display
    peAk = 0;
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
	} else {
		globalTrig = false;
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
    
} // end of update function

//--------------------------------------------------------------
void ofApp::draw() {

    // Re-draw background whenever persistence is disabled
    if (persistEnabled) {
        persistFbo.begin();
        // Clear any remaining artifacts from GPU
        if (persistFirstRender) {
            persistFirstRender = false;
            ofClear(255, 255, 255, 0);
        }
    }
    
    // set the audio buffer
    lua.setNumberVector("inL", left);
    //lua.setNumberVector("inR", right);
    
    // enable depth
    ofEnableDepthTest();
    
    // draw the lua mode
    ofPushMatrix();
  	lua.scriptDraw();
    ofPopMatrix();
    
    // disable depth
    ofDisableDepthTest();
    
    //ofSetColor(255);
    //ofDrawBitmapString( ofGetFrameRate(), 300, 300);

    if (osdEnabled == true) {
	// draw it
	int space = ceil( ofGetWidth() / 10 );
	ofTranslate( space, ceil( ofGetHeight() / 24 ) );
	drawTheOsd();
   }

   if (persistEnabled) {
        persistFbo.end();
        persistFbo.draw(0,0);
   }

}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
   
    if (dummyAudio == 0){	
        for (size_t i = 0; i < input.getNumFrames(); i++){
            left[i]  = input[i*2] * globalGain;
            //right[i] = input[i*2+1] * globalGain;
	    
        }
    } else {
        for (size_t i = 0; i < input.getNumFrames(); i++){
            left[i]  = sin((i*TWO_PI)/input.getNumFrames());
            //right[i] = cos((i*TWO_PI)/input.getNumFrames());
        }
    }
       
}

//--------------------------------------------------------------
void ofApp::exit() {
    // call the script's exit() function
    lua.scriptExit();
    thread.stopThread();
    
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

// up script
void ofApp::nextScript() {
    currentScript++;
    if(currentScript > sizeScripts-1) {
        currentScript = 0;
    }
    reloadScript();
}

// down script
void ofApp::prevScript() {
    if(currentScript == 0) {
        currentScript = sizeScripts-1;
    }
    else {
        currentScript--;
    }
    reloadScript();
}

// recall for the scene
void ofApp::recallScript(string num) {
	
	// search scripts for matching string might be a heavy process on the cpu
	int wow = 0;
	for (int i=0; i<countPaths; i++) {
		if( scripts[i] == num ) {
			wow = i;
		}
	}
	currentScript = wow;
	reloadScript();
}

// send the current mode for scene saving in pure data
void ofApp::sendCurrentScript(string name) {
	// compose the message
	mess.setAddress( "/currentScript" );
	mess.addStringArg( name );
	sender.sendMessage( mess );
	mess.clear();
}


// osd draw function 
void ofApp::drawTheOsd()  {
		
		
		int fontHeight = floor( osdFont.stringHeight( "Lpl" ) + 4) ;	
		int spaceTrack = floor( fontHeight/2 );
		
		// mode name
		ofPushMatrix();
			ofTranslate(0,0);
			std::stringstream scrpz;
			scrpz << "Mode: " << modeTitle;
    			int scrpW = ceil( osdFont.stringWidth( scrpz.str() ));
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,scrpW+4,fontHeight);
			ofSetColor(255);
			osdFont.drawString(scrpz.str(), 2, fontHeight-4 );
		

		// Explain the Mode 
			ofTranslate(0, fontHeight + spaceTrack );
			std::stringstream eXplain;
			eXplain << "Mode Description: " << modeDescrip;
			int eXwith = ceil (osdFont.stringWidth( eXplain.str() ));
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,eXwith+4,fontHeight);
			
			ofSetColor(255);
			osdFont.drawString( eXplain.str(), 2, fontHeight-4);

		// knobs
		
			// draw background
			int knobW = floor(osdW/38);
			int knobH = ceil(osdH/5.4);
			int knobTextH = floor(osdH/68);
			ofFill();
			ofTranslate(0, fontHeight + spaceTrack);
			ofSetColor(0);
			ofDrawRectangle(0,0,knobW*16,knobH+(knobTextH*3));
			
			
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
	
		// volume
			ofTranslate(0, (knobH+(knobTextH*3)) + spaceTrack ) ;
			int volChunk = floor(osdW/160);
			std::stringstream inputStr;
			inputStr <<  "Input Level: ";
			int visVol = floor( (peAk * 16.0)+0.4999 );
			int volStrWidth = ceil( osdFont.stringWidth( inputStr.str() ) );
			
			ofSetColor(0);
			ofDrawRectangle(0,0,volStrWidth+(volChunk*20), volChunk*6 );
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
			
				
		// Trigger
		
			ofTranslate(0,spaceTrack + (volChunk*6) );
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,knobW*4,knobW+(volChunk*2));
			ofSetColor(255);
			osdFont.drawString( "Trigger: ", 2, fontHeight+2);
			
			if (globalTrig == true) {
				ofSetColor(255,255,0);
				ofFill();
				ofDrawRectangle( knobW*2, volChunk, knobW, knobW);
			} else {
			}
			// draw white border
			ofNoFill();
			ofSetColor(255);
			ofDrawRectangle( knobW*2,volChunk , knobW, knobW);
		
		// midi
			int chunk = ceil(osdH/108);
			ofTranslate(0, spaceTrack + (knobW+(volChunk*2)) );
			
			
			std::stringstream midStr;
			midStr << "MIDI: ";
			int midiW = osdFont.stringWidth( midStr.str() );
			
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,(midiW+(chunk*20)),(chunk*9));
			ofSetColor(255);
			osdFont.drawString( midStr.str(), 2, fontHeight+2);
			osdFontK.drawString( "0", midiW, ceil(chunk*1.5) );
			osdFontK.drawString( "127", (midiW+(chunk*17))+2 ,(chunk/2)+(chunk*8) );
			for ( int i=0; i<9; i++) {
				// draw horizontal lines
				int yPos = (i*chunk) + ceil(chunk/2);
				ofDrawLine(midiW+chunk,yPos,(midiW+chunk)+ceil(chunk*16),yPos);
			}
			for (int i=0; i<17; i++) {
				// draw vertical lines
				int xPos = (i*chunk) + (midiW+chunk);
				ofDrawLine(xPos,ceil(chunk/2),xPos,ceil(chunk/2)+ceil(chunk*8));
			}
			for(int i=0; i<128; i++) {
				if (midiTable[i] != 0) {
					int xPos = ((i % 16) * chunk)+(midiW+chunk);
					int yPos = (floor( i / 16 ) * chunk) + ceil(chunk/2); 
					ofSetColor(0,255,255);
					ofFill();
					ofDrawRectangle(xPos,yPos, chunk, chunk);
				}
			}
		
		
		// Sequencer 
		
			ofTranslate(0, spaceTrack + (chunk*9) );
			std::stringstream seQ;
			
			if( seqStatus == 1) seQ << "Sequencer: " << "Ready To Record";
			if( seqStatus == 2) seQ << "Sequencer: " << "Recording";
			if( seqStatus == 3) seQ << "Sequencer: " << "Playing";
			if( seqStatus == 0) seQ << "Sequencer: " << "Stopped";
			
			int seqW = osdFont.stringWidth( seQ.str() );
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,seqW+4,fontHeight);
			ofSetColor(255);
			osdFont.drawString( seQ.str(), 2, fontHeight-4);


		//WIFI
			ofTranslate(0,spaceTrack+fontHeight);
			std::stringstream wifI;
			wifI << "WIFI: " << theWifiName;
			ofFill();
			ofSetColor(0);
			int wifiW = osdFont.stringWidth( wifI.str() );
			ofDrawRectangle(0,0,wifiW+4, fontHeight);
			ofSetColor(255);
			osdFont.drawString(wifI.str(), 2, fontHeight-4);
		

		//IP
		
			ofTranslate(0,spaceTrack+fontHeight);
			std::stringstream ipAd;
			ipAd << "IP Address: " << theIP;
			ofFill();
			ofSetColor(0);
			int ipW = osdFont.stringWidth( ipAd.str() );
			ofDrawRectangle(0,0,ipW+4, fontHeight);
			
			ofSetColor(255);
			osdFont.drawString(ipAd.str(), 2, fontHeight-4);
		

		//OS Version
		
			ofTranslate(0,spaceTrack + fontHeight );
			
			std::stringstream osStr;
			osStr << "OS Version: " << osVersion;
			ofFill();
			ofSetColor(0);
			int osW = osdFont.stringWidth( osStr.str() );
			ofDrawRectangle(0,0,osW+4, fontHeight);
			
			ofSetColor(255);
			osdFont.drawString(osStr.str(), 2, fontHeight-4);
	
			
		
		// FPS
			ofTranslate(0,spaceTrack+fontHeight);
			std::stringstream fPs;
			int getFramz = int( ofGetFrameRate() );
			
			fPs << "FPS: " << getFramz ;
			int fpsW = ceil( osdFont.stringWidth( fPs.str() ));
			ofSetColor(0);
			ofDrawRectangle(0,0,fpsW+4,fontHeight);
			
			ofSetColor(255);
			osdFont.drawString(fPs.str(), 2, fontHeight-4);
		
		// Scene Stuff
			ofTranslate(0, spaceTrack+fontHeight);
			std::stringstream scenE;
			if(globalScene == 0) {
				scenE << "Scene: " << "None" << " of " << totalScenes;
			} else {

				scenE << "Scene: " << globalScene << " of " << totalScenes;
			}
			int sceneW = ceil( osdFont.stringWidth( scenE.str() ) );
			ofSetColor(0);
			ofDrawRectangle(0,0,sceneW+4, fontHeight);
			ofSetColor(255);
			osdFont.drawString(scenE.str(), 2, fontHeight-4);
		
		
		// Screen Grabs
		
			ofTranslate(0, spaceTrack+fontHeight);
			std::stringstream graBz;
			graBz << "Total Screen Grabs: " << grabCounter;	
			int grabW = ceil( osdFont.stringWidth( graBz.str() ));
			ofSetColor(0);
			ofDrawRectangle(0,0,grabW+4, fontHeight);
			ofSetColor(255);
			osdFont.drawString(graBz.str(), 2, fontHeight-4);
		ofPopMatrix();

		// draw the last 5 screen grabs
		
		ofPushMatrix();
			int grabHeit = ceil(osdH/10);
			ofTranslate( ceil(osdW/2), ceil(osdH/3) );
			
			if( grabCounter >= 1 ) {
				grab1.resize( ceil(osdW/10), grabHeit);
				grab1.draw(0,0);
				ofTranslate( 0, grabHeit + (grabHeit/3) );
			}
			if( grabCounter >= 2 ) {
				grab2.resize( ceil(osdW/10), ceil(osdH/10) );
				grab2.draw(0,0);
				ofTranslate( 0, grabHeit + (grabHeit/3) );
			}
			if( grabCounter >= 3 ) {
				grab3.resize( ceil(osdW/10), ceil(osdH/10) );
				grab3.draw(0,0);
				ofTranslate( 0, grabHeit + (grabHeit/3) );
			}
			if( grabCounter >= 4 ) {
				grab4.resize( ceil(osdW/10), ceil(osdH/10) );
				grab4.draw(0,0);
			}	
			
		ofPopMatrix();
			
		
		// draw the shift options if osd and shift is on
		if (shIft == true) {
			ofPushMatrix();
				// gain knob1
				ofTranslate(osdW/2,0);
				std::stringstream gAin;
				gAin << "Gain: " << ceil(globalGain*100) << "%";
    				int gW = ceil( osdFont.stringWidth( gAin.str() ));
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
				int trigW = ceil( osdFont.stringWidth( trigPut.str() ));
				ofSetColor(0);
				ofFill();
				ofDrawRectangle(0,0,trigW+4,fontHeight);
				
				ofSetColor(255);
				osdFont.drawString( trigPut.str(),2,fontHeight-4);

				// Midi channel select
				ofTranslate(0,fontHeight*2);
				
				std::stringstream mIdChan;
				mIdChan << "MIDI Channel: " << globalMidiChannel;
				int mIdW = ceil( osdFont.stringWidth( mIdChan.str() ));
				ofSetColor(0);
				ofFill();
				ofDrawRectangle(0,0,mIdW+4,fontHeight);
				ofSetColor(255);
				osdFont.drawString( mIdChan.str(),2,fontHeight-4);

			ofPopMatrix();

		}
}// end the function
