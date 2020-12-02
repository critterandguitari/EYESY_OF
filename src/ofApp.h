/*
 * Copyright (c) 2020 Owen Osborn, Critter & Gutiari, Inc.
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */
#pragma once

#include "ofMain.h"
#include "ofxLua.h"
#include "ofxOsc.h"
#include "ofxMidi.h"
//#include "ofxGPIO.h"
#include <queue>

#define PORT 4000

class ofApp : public ofBaseApp, ofxLuaListener, ofxMidiListener {

	public:

		// main
		void setup();
		void update();
		void draw();
		void exit();
		
		// input
		void keyPressed(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		
		// ofxLua error callback
		void errorReceived(std::string& msg);
		
		// script control
		void reloadScript();
		void nextScript();
		void prevScript();
	
		ofxLua lua;
		vector<string> scripts;
		size_t currentScript;

		// osc control
		ofxOscReceiver receiver;

		// audio stuff
		void audioIn(ofSoundBuffer & input);

		// MIDI In
		void newMidiMessage(ofxMidiMessage& eventArgs);
	
		ofxMidiIn midiIn;
		std::queue<ofxMidiMessage> midiMessages;
		std::size_t maxMessages = 32; //< max number of messages to keep track of
	
		vector <lua_Number> left;
		vector <lua_Number> right;
		vector <float> volHistory;
		
		int 	bufferCounter;
		int 	drawCounter;
		
		float smoothedVol;
		float scaledVol;
		float peak;
		
		ofSoundStream soundStream;

		int                 snapCounter;
		string              snapString;
		ofImage             img;
	
		ofFbo fbo;
		int persistSetting;
		bool osdSetting;
		bool osdShiftSetting;
		string modesPath;
		string grabsPath;
		
		//GPIO gpio25;
		//int state_button2;
};
