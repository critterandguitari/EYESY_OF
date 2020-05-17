/*
 * Copyright (c) 2012 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/danomatika/ofxLua for documentation
 *
 */
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    
	
    // listen on the given port
	cout << "listening for osc messages on port " << PORT << "\n";
	receiver.setup(PORT);	
    
    ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetLogLevel("ofxLua", OF_LOG_VERBOSE);
		

    ofHideCursor();

    // scripts to run
	scripts.push_back("/sdcard/Modes/oFLua/Basic Scope/main.lua");
/*	scripts.push_back("/sdcard/lua/graphicsExample.lua");
	scripts.push_back("/sdcard/lua/imageLoaderExample.lua");
	scripts.push_back("/sdcard/lua/polygonExample.lua");
	scripts.push_back("/sdcard/lua/fontsExample.lua");
	scripts.push_back("/sdcard/lua/owen.lua");
	scripts.push_back("/sdcard/lua/chris.lua");
	scripts.push_back("/sdcard/lua/knobsExample.lua");
	scripts.push_back("/sdcard/lua/script2.lua");
	scripts.push_back("/sdcard/lua/script3.lua");
	scripts.push_back("/sdcard/lua/script4.lua");*/
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
	
    // setup audio
	soundStream.printDeviceList();
	
	int bufferSize = 256;

	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	volHistory.assign(400, 0.0);
	
	bufferCounter	= 0;
	drawCounter		= 0;
	smoothedVol     = 0.0;
	scaledVol		= 0.0;

	ofSoundStreamSettings settings;
	
    // device by name
	auto devices = soundStream.getMatchingDevices("default");
	if(!devices.empty()){
		settings.setInDevice(devices[0]);
	}

	settings.setInListener(this);
	settings.sampleRate = 22050;
	settings.numOutputChannels = 0;
	settings.numInputChannels = 2;
	settings.bufferSize = bufferSize;
	soundStream.setup(settings);    
    
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
            if (m.getArgAsFloat(0) == 4 && m.getArgAsFloat(1) > 0) {
                cout << "back" << "\n";
                prevScript();
            }
            if (m.getArgAsFloat(0) == 2 && m.getArgAsFloat(1) > 0) {
                cout << "fwd" << "\n";
                nextScript();
            }
            if (m.getArgAsFloat(0) == 5 && m.getArgAsFloat(1) > 0) {
                img.grabScreen(0,0,ofGetWidth(),ofGetHeight());
                string fileName = "snapshot_"+ofToString(10000+snapCounter)+".png";
                cout << "saving " + fileName + "...";
                img.save("/sdcard/Grabs/" + fileName);
                cout << "saved\n";
                snapCounter++;
            }
        }
        if(m.getAddress() == "/knobs") {
            lua.setNumber("knob1", m.getArgAsInt32(4));
            lua.setNumber("knob2", m.getArgAsInt32(2));
            lua.setNumber("knob3", m.getArgAsInt32(0));
            lua.setNumber("knob4", m.getArgAsInt32(1));
            lua.setNumber("knob5", m.getArgAsInt32(3));
        }
	}
	
    // call the script's update() function
	lua.scriptUpdate();

}

//--------------------------------------------------------------
void ofApp::draw() {
    
    lua.setNumberVector("inL", left);
    lua.setNumberVector("inR", right);
    
	// call the script's draw() function
    lua.scriptDraw();
    
	/*ofSetColor(0);
	ofDrawBitmapString("use <- & -> to change between scripts", 10, ofGetHeight()-22);
	ofDrawBitmapString(scripts[currentScript], 10, ofGetHeight()-10);*/

}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
	
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;	

	for (size_t i = 0; i < input.getNumFrames(); i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;
		//curVol += left[i] * left[i];
		//curVol += right[i] * right[i];
		//numCounted+=2;
	}
	
	//this is how we get the mean of rms :) 
	//curVol /= (float)numCounted;
	
	// this is how we get the root of rms :) 
	//curVol = sqrt( curVol );
	
	//smoothedVol *= 0.93;
	//smoothedVol += 0.07 * curVol;
    

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
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	lua.scriptMouseMoved(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	lua.scriptMouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	lua.scriptMousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	lua.scriptMouseReleased(x, y, button);
}

//--------------------------------------------------------------
void ofApp::errorReceived(std::string& msg) {
	ofLogNotice() << "got a script error: " << msg;
}

//--------------------------------------------------------------
void ofApp::reloadScript() {
	// exit, reinit the lua state, and reload the current script
	lua.scriptExit();
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
