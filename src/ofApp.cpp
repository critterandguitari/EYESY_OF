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

	// Workaround for "other" sized screens/displays
	ofSetWindowShape(ofGetWidth(), ofGetHeight());
	ofSetWindowPosition(0, 0);

	// listen on the given port
	cout << "listening for osc messages on port " << PORT << "\n";
	receiver.setup(PORT);	
	
	//ofSetVerticalSync(true); // this will force to 60 fps
	ofSetFrameRate(30);
	ofSetLogLevel("ofxLua", OF_LOG_VERBOSE);

	ofHideCursor();

	ofSetBackgroundColor(0,0,0);

	// GPIO
    //gpio25.setup(GPIO24,IN,LOW);
	//gpio25.export_gpio();
	//lua.setNumber("button2", 0);
	
	// MIDI
	// print input ports to console
	midiIn.listInPorts();
	
	// Open a MIDI Port
		//midiIn.openPort(0); // open port by number (you may need to change this)
		//midiIn.openPort("IAC Pure Data In");	// by name
		midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
	
		// don't ignore sysex, timing, & active sense messages (midiSysex, midiTiming, midiSense),
		// these are ignored by default
		//midiIn.ignoreTypes(false, false, false);
	
	// add ofApp as a listener
	midiIn.addListener(this);
	
	// print received messages to the console
	//midiIn.setVerbose(true);


	// AUDIO Setup 
	soundStream.printDeviceList();
	
	int bufferSize = 256;

	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	
	bufferCounter	= 0;

	ofSoundStreamSettings settings;
	
	// device by name
	auto devices = soundStream.getMatchingDevices("default");
	if(!devices.empty()){
		settings.setInDevice(devices[0]);
	}

	settings.setInListener(this);
    settings.sampleRate = 11025;
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
		if (dir.doesDirectoryExist(dir.getPath(i))) {
			ofLogNotice(dir.getPath(i) + "/main.lua");
			scripts.push_back(dir.getPath(i) + "/main.lua");
		}
	}

	persistSetting = 0;
	osdSetting = false;
	osdShiftSetting = false;
	
	// scripts to run
	currentScript = 0;
	
	// init the lua state
	lua.init(true); // true because we want to stop on an error
	
	// listen to error events
	lua.addListener(this);

	lua.setBool("osd_state", false);
	lua.setBool("osd_shift", false);
	
	// run a script
	// true = change working directory to the script's parent dir
	// so lua will find scripts with relative paths via require
	// note: changing dir does *not* affect the OF data path
	lua.doScript(scripts[currentScript]);  //lua.doScript(scripts[currentScript], true);
	
	// call the script's setup() function
	lua.scriptSetup();
}

//--------------------------------------------------------------
void ofApp::update() {
	//gpio25.getval_gpio(state_button2);
	//lua.setNumber("button2", state_button2);
	
	// Check for MIDI Messages and set lua table
    while (!midiMessages.empty()) {
    	lua.pushTable("midi_msg");
    	
        // printf("%lu messages to process.\n", midiMessages.size());
        ofxMidiMessage message = midiMessages.front();

		if(message.status < MIDI_SYSEX) {
			lua.setNumber(1, message.status);
			lua.setNumber(2, message.channel);
			lua.setNumber(3, message.pitch);
			lua.setNumber(4, message.velocity);
			lua.setNumber(5, message.control);
			lua.setNumber(6, message.value);
			lua.setNumber(7, message.portNum);
			lua.setString(8, message.portName);
			//lua.setNumber(9, message.deltatime);
			//lua.setNumber(10, message.bytes);
			lua.popTable();
		}

        //std::cout << "Pitch: " << midimes.pitch << std::endl;
        midiMessages.pop();
    }

	// check for waiting OSC messages
	while(receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(m);
		//cout << "new message on port " << PORT << m.getAddress() << "\n";

		// Additions for TouchOSC remote control
		if(m.getAddress() == "/shift") {	
			//cout << "shift" << "\n";
			osdShiftSetting = !osdShiftSetting;
			lua.setBool("osd_shift", osdShiftSetting);
		}
		if(m.getAddress() == "/key/1") {	
			//cout << "osd" << "\n";
			if (m.getArgAsInt32(0) == 1){
				osdSetting = !osdSetting;
                lua.setBool("osd_state", osdSetting);
				if (osdShiftSetting && osdSetting) {
					osdShiftSetting = false;
					lua.setBool("osd_shift", osdShiftSetting);
				}
			}
		}
		if(m.getAddress() == "/key/3") {	
			//cout << "change persist" << "\n";
			persistSetting++;
			persistSetting &= 1;
			if (persistSetting == 1){
				lua.setBool("persist", true);
			}else{
				lua.setBool("persist", false);
			}
		}
		if(m.getAddress() == "/key/4") {	 
			//cout << "back" << "\n";
			prevScript();
		}
		if(m.getAddress() == "/key/5") {	 
			//cout << "fwd" << "\n";
			nextScript();
		}
		if(m.getAddress() == "/key/10") {	 
			//cout << "trig" << "\n";
			if (m.getArgAsInt32(0) == 1){
				lua.setBool("trig", true);
			}else{
				lua.setBool("trig", false);
			}
		}
		// EYESY Hardware
		if(m.getAddress() == "/key") {	 
			if (m.getArgAsInt32(0) == 1 && m.getArgAsInt32(1) > 0) {
				// placeholder for OSD
				//cout << "on screen display" << "\n";
				osdSetting = !osdSetting;
                lua.setBool("osd_state", osdSetting);
			}
			if (m.getArgAsInt32(0) == 2 && m.getArgAsInt32(1) > 0) {
				// placeholder for shift
				//cout << "shift" << "\n";
				osdShiftSetting = !osdShiftSetting;
				lua.setBool("osd_shift", osdShiftSetting);
				
			}
			// mode selectors
			if (m.getArgAsInt32(0) == 4 && m.getArgAsInt32(1) > 0) {
				//cout << "back" << "\n";
				prevScript();
			}
			if (m.getArgAsInt32(0) == 5 && m.getArgAsInt32(1) > 0) {
				//cout << "fwd" << "\n";
				nextScript();
			}
			// Scene selectors
			if (m.getArgAsInt32(0) == 6 && m.getArgAsInt32(1) > 0) {
				// placeholder for scene back
			}
			if (m.getArgAsInt32(0) == 7 && m.getArgAsInt32(1) > 0) {
				// placeholder for scene forward
			}
			if (m.getArgAsInt32(0) == 8 && m.getArgAsInt32(1) > 0) {
				// placeholder for scene save
			}
			if (m.getArgAsInt32(0) == 9 && m.getArgAsInt32(1) > 0) {
				img.grabScreen(0,0,ofGetWidth(),ofGetHeight());
				string fileName = "snapshot_"+ofToString(10000+snapCounter)+".png";
				//cout << "saving " + fileName + "...";
				img.save("/sdcard/Grabs/" + fileName);
				//cout << "saved\n";
				snapCounter++;
			}
			if (m.getArgAsInt32(0) == 3 && m.getArgAsInt32(1) > 0) {
				//cout << "change persist" << "\n";
				persistSetting++;
				persistSetting &= 1;
			}
			if (m.getArgAsInt32(0) == 10 && m.getArgAsInt32(1) > 0) {
				//cout << "trig" << "\n";
				lua.setBool("trig", true);
			} else if (m.getArgAsInt32(0) == 10 && m.getArgAsInt32(1) == 0) {
				lua.setBool("trig", false);
			}

		}
		// Additions for TouchOSC remote control
		if(m.getAddress() == "/knobs/1") {
			lua.setNumber("knob1", (float)m.getArgAsInt32(0) / 1023);
			//printf("%lu knob1.\n", m.getArgAsInt32(0));
		}
		if(m.getAddress() == "/knobs/2") {
			lua.setNumber("knob2", (float)m.getArgAsInt32(0) / 1023);
		}
		if(m.getAddress() == "/knobs/3") {
			lua.setNumber("knob3", (float)m.getArgAsInt32(0) / 1023);
		}
		if(m.getAddress() == "/knobs/4") {
			lua.setNumber("knob4", (float)m.getArgAsInt32(0) / 1023);
		}
		if(m.getAddress() == "/knobs/5") {
			lua.setNumber("knob5", (float)m.getArgAsInt32(0) / 1023);
		}
		
		// EYESY Hardware
		if(m.getAddress() == "/knobs") {
			lua.setNumber("knob1", (float)m.getArgAsInt32(0) / 1023);
			lua.setNumber("knob2", (float)m.getArgAsInt32(1) / 1023);
			lua.setNumber("knob3", (float)m.getArgAsInt32(2) / 1023);
			lua.setNumber("knob4", (float)m.getArgAsInt32(3) / 1023);
			lua.setNumber("knob5", (float)m.getArgAsInt32(4) / 1023);
		}
		if(m.getAddress() == "/reload") {
			//cout << "reloading\n";
			reloadScript();
		}
		if(m.getAddress() == "/ascale") {
			//cout << "audio scale\n";
			lua.setNumber("ascale", (float)m.getArgAsInt32(0) / 1023); // float 0 to 2 from osc
		}
		if(m.getAddress() == "/trigger_source") {
			//cout << "trigger source\n";
			lua.setNumber("trigsource", m.getArgAsInt32(0)); // 1 to 6 from osc
		}
		if(m.getAddress() == "/midi_ch") {
			//cout << "midi channel\n";
			lua.setNumber("midi_ch", m.getArgAsInt32(0)); // 1 to 16 from osc

		}
	}
	
	// call the script's update() function
	lua.scriptUpdate();

}

//--------------------------------------------------------------
void ofApp::draw() {
	
	lua.setNumberVector("inL", left);
	lua.setNumberVector("inR", right);
	lua.setNumber("peak", peak);

	lua.scriptDraw();
	
	//ofDrawBitmapString(scripts[currentScript], 10, ofGetHeight()-10);

	// Clear flags	  
	lua.setBool("trig", false);
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
	
	peak = 0.0;
	
	// samples are "interleaved"
	//int numCounted = 0; 

	for (size_t i = 0; i < input.getNumFrames(); i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;
		if (left[i] > peak ) {
			peak = left[i];
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

	// midi clean up
	midiIn.closePort();
	midiIn.removeListener(this);
	
	// GPIO cleanup
	//gpio25.unexport_gpio();
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {

	// add the latest message to the message queue
	midiMessages.push(msg);


		
	// remove any old messages if we have too many
 
	//while(midiMessages.size() > maxMessages) {

	//	midiMessages.erase(midiMessages.begin());
	//}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	
	switch(key) {
		case '?':
			midiIn.listInPorts();
			break;

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
			//cout << "fps: " << ofGetFrameRate() << "\n";	
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
	
	// init OF
	ofSetupScreen();
	ofSetupGraphicDefaults();

	// load new
	lua.init();
	lua.doScript(scripts[currentScript]); //lua.doScript(scripts[currentScript], true);
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
