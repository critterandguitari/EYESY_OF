/*
 * Copyright (c) 2020 Owen Osborn, Critter & Gutiari, Inc.
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
   
    // listen on the given port
    cout << "listening for osc messages on port " << PORT << "\n";
    receiver.setup(PORT);    
    
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
    for(int i = 0; i < dir.size(); i++){
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

    // osd setup
    osdFont.load("CGFont_0.18.otf", 24, true, true, true, 10, 64);
    osdFont.setLetterSpacing(1);
    osdFontK.load("CGFont_0.18.otf", 16, true, true, true, 10, 64);
    osdFontK.setLetterSpacing(1);

    osdEnabled = 0;
    osdFbo.allocate(900, 1000);
    dummyAudio = 0;
}

//--------------------------------------------------------------
void ofApp::update() {

    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        //cout << "new message on port " << PORT << m.getAddress() << "\n";
        if(m.getAddress() == "/key") {   
            if (m.getArgAsInt32(0) == 4 && m.getArgAsInt32(1) > 0) {
                cout << "back" << "\n";
                prevScript();
            }
            if (m.getArgAsInt32(0) == 5 && m.getArgAsInt32(1) > 0) {
                cout << "fwd" << "\n";
                nextScript();
            }
            if (m.getArgAsInt32(0) == 9 && m.getArgAsInt32(1) > 0) {
                img.grabScreen(0,0,ofGetWidth(),ofGetHeight());
                string fileName = "snapshot_"+ofToString(10000+snapCounter)+".png";
                cout << "saving " + fileName + "...";
                img.save("/sdcard/Grabs/" + fileName);
                cout << "saved\n";
                snapCounter++;
            }
            if (m.getArgAsInt32(0) == 10 && m.getArgAsInt32(1) == 0) dummyAudio = 0;
            if (m.getArgAsInt32(0) == 10 && m.getArgAsInt32(1) > 0) {
		dummyAudio = 1;
                cout << "trig" << "\n";
                lua.setBool("trig", true);
            }
	    if (m.getArgAsInt32(0) == 3 && m.getArgAsInt32(1) > 0) {
            	cout << "change persist" << "\n";
            } 
       	    if (m.getArgAsInt32(0) == 1 && m.getArgAsInt32(1) > 0) {
		osdEnabled = osdEnabled ? 0 : 1;
            	cout << "change OSD: " << osdEnabled << "\n";
            } 

	
	}
        if(m.getAddress() == "/knob1") lua.setNumber("knob1", (float)m.getArgAsInt32(0) / 1023);
        if(m.getAddress() == "/knob2") lua.setNumber("knob2", (float)m.getArgAsInt32(0) / 1023);
        if(m.getAddress() == "/knob3") lua.setNumber("knob3", (float)m.getArgAsInt32(0) / 1023);
        if(m.getAddress() == "/knob4") lua.setNumber("knob4", (float)m.getArgAsInt32(0) / 1023);
        if(m.getAddress() == "/knob5") lua.setNumber("knob5", (float)m.getArgAsInt32(0) / 1023);
	/*if(m.getAddress() == "/knobs") {
            lua.setNumber("knob1", (float)m.getArgAsInt32(0) / 1023);
            lua.setNumber("knob2", (float)m.getArgAsInt32(1) / 1023);
            lua.setNumber("knob3", (float)m.getArgAsInt32(2) / 1023);
            lua.setNumber("knob4", (float)m.getArgAsInt32(3) / 1023);
            lua.setNumber("knob5", (float)m.getArgAsInt32(4) / 1023);
        }*/
	if(m.getAddress() == "/reload") {
            cout << "reloading\n";
            reloadScript();
        }
    }
    
    // call the script's update() function
    lua.scriptUpdate();

}

//--------------------------------------------------------------
void ofApp::draw() {
    
    lua.setNumberVector("inL", left);
    lua.setNumberVector("inR", right);
    
    lua.scriptDraw();
    // clear flags    
    lua.setBool("trig", false);

    /*ofClear(255);
    ofFill();
    ofSetColor(0);
    ofDrawRectangle(500,500,100,100);
    ofSetColor(255,0,0);
    ofDrawRectangle(600,600,100,100);
    ofNoFill();
    */

    // OSD 
    if (osdEnabled) {
	
	// begin the fbo
	osdFbo.begin();
		ofClear(255);
		// mode name
		std::stringstream scrpz;
		scrpz << "Mode: " << lua.getString("modeTitle");
    		float scrpW = osdFont.stringWidth( scrpz.str() );
		ofPushMatrix();
			ofTranslate(0,10);
			ofSetColor(0);
			ofFill();
			ofDrawRectangle(0,0,scrpW+4,26);
			ofSetColor(255);
			osdFont.drawString(scrpz.str(), 2, 20);
		ofPopMatrix();
		
		// FPS
		ofPushMatrix();
			ofTranslate(0,50);
			std::stringstream fPs;
			fPs << "FPS: " << ofGetFrameRate() ;
			float fpsW = osdFont.stringWidth( fPs.str() );
			ofSetColor(0);
			ofDrawRectangle(0,0,fpsW+4,26);
			ofSetColor(255);
			osdFont.drawString(fPs.str(), 2, 20);
		ofPopMatrix();
		    		
		// knobs
		ofPushMatrix();
			// draw background
			ofFill();
			ofTranslate(0,100); // 100px down
			ofSetColor(0);
			ofDrawRectangle(0,0,775,250);
			// draw k1
			ofPushMatrix();
				ofTranslate(25, 25);
				ofSetColor(255);
				ofDrawRectangle(0,0,50,200);
				std::stringstream k1Name;
				k1Name << lua.getString("titleK1");
				osdFontK.drawString( k1Name.str(), 0, 220);
				ofSetColor(0);
				ofDrawRectangle(1,1,48,(1-lua.getNumber("knob1"))*198 );
			ofPopMatrix();
			// draw k2
			ofPushMatrix();
				ofTranslate(175, 25);
				ofSetColor(255);
				ofDrawRectangle(0,0,50,200);
				std::stringstream k2Name;
				k2Name << lua.getString("titleK2");
				osdFontK.drawString( k2Name.str(), 0, 220);
				ofSetColor(0);
				ofDrawRectangle(1,1,48,(1-lua.getNumber("knob2"))*198 );
			ofPopMatrix();
			// draw k3
			ofPushMatrix();
				ofTranslate(325, 25);
				ofSetColor(255);
				ofDrawRectangle(0,0,50,200);
				std::stringstream k3Name;
				k3Name << lua.getString("titleK3");
				osdFontK.drawString( k3Name.str(), 0, 220);
				ofSetColor(0);
				ofDrawRectangle(1,1,48,(1-lua.getNumber("knob3"))*198 );
			ofPopMatrix();
			// draw k4
			ofPushMatrix();
				ofTranslate(475, 25);
				ofSetColor(255);
				ofDrawRectangle(0,0,50,200);
				std::stringstream k4Name;
				k4Name << lua.getString("titleK4");
				osdFontK.drawString( k4Name.str(), 0, 220);
				ofSetColor(0);
				ofDrawRectangle(1,1,48,(1-lua.getNumber("knob4"))*198 );
			ofPopMatrix();
			// draw k5
			ofPushMatrix();
				ofTranslate(625, 25);
				ofSetColor(255);
				ofDrawRectangle(0,0,50,200);
				std::stringstream k5Name;
				k5Name << lua.getString("titleK5");
				osdFontK.drawString( k5Name.str(), 0, 220);
				ofSetColor(0);
				ofDrawRectangle(1,1,48,(1-lua.getNumber("knob5"))*198 );
			ofPopMatrix();
		ofPopMatrix();
		
		// Trigger
		ofPushMatrix();
			ofTranslate(0,375);
			ofSetColor(0);
			ofDrawRectangle(0,0,250, 50);
			ofSetColor(255);
			osdFont.drawString( "Trigger: ", 2, 26 );
		ofPopMatrix();	
		
		// midi

	// end the fbo
	osdFbo.end();
	// draw it
	ofSetColor(255);
	ofTranslate(40,20,10);
	osdFbo.draw(0,0);
	
    }

}
//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
   
    if (!dummyAudio){	
        for (size_t i = 0; i < input.getNumFrames(); i++){
            left[i]  = input[i*2];
            right[i] = input[i*2+1];
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
    if(currentScript > scripts.size()-1) {
        currentScript = 0;
    }
    reloadScript();
}

void ofApp::prevScript() {
    if(currentScript == 0) {
        currentScript = scripts.size()-1;
    }
    else {
        currentScript--;
    }
    reloadScript();
}

