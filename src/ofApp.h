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

#define PORT 4000


class osdThread : public ofThread {
        bool fetchIpWifi;	
	public:
		string wifi;
		string ip;	

	void threadedFunction() {
		while( isThreadRunning() ) {
			
			//start
			
			if( fetchIpWifi == true) {	
				// lock so other thread doesnt accesss
				lock();
					// get IP address
					string str = ofSystem( "ifconfig" );
					int place = str.find("wlan0",0);
					string str2 = str.substr(place+5, 200);
					int place2 = str2.find("inet", 0);
					ip =  str2.substr(place2+5,13);
				
					// get wifi
					wifi = ofSystem( "iwgetid -r");

				unlock();
			} else {
				sleep(5000);
			}
		}	
		
	}	
		
};



class ofApp : public ofBaseApp, ofxLuaListener {

    public:
	
	osdThread thread;

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
	void recallScript(int num);
	void sendCurrentScript(int cur);
	void updateScreenGrabs();
	void drawTheOsd();
    
        ofxLua 		lua;
        vector<string> 	scripts;
        size_t 		currentScript;
	bool 		fetchIpWifi;

        // osc control
        ofxOscReceiver 	receiver;
	ofxOscSender 	sender;
	
	ofImage 	grab1;
	ofImage 	grab2;
	ofImage 	grab3;
	ofImage 	grab4;

	
        // audio stuff
        void audioIn(ofSoundBuffer & input);
    
        vector <lua_Number> left;
        vector <lua_Number> right;

        int     	bufferCounter;
        int     	drawCounter;
    
        float 		smoothedVol;
	int 		fontHeight;
	int 		marg;
        float 		scaledVol;
        
        ofSoundStream 	soundStream;

        int        	snapCounter;
        string          snapString;
        ofImage         img;

	bool 		osdEnabled;
	int 		midiTable[128];
	int 		totalScenes;
	string 		osVersion;
	ofxOscMessage 	mess;
	size_t 		sizeScripts;
	
		
	vector<int> 	osdMidi{0,0};
	
	// osd stuff
	ofFbo 		osdFbo;
	ofFbo		smallFbo;
	ofTrueTypeFont 	osdFont;
	ofTrueTypeFont 	osdFontK;
	int 		dummyAudio;
	int 		seqStatus;
	int 		osdCount;
	float 		osdW;
	float 		osdH;
	float 		globalGain = 1;
	int 		globalTrigInput;
	int 		globalMidiChannel;
	int 		globalScene;
        //bool 		globalTrig;
	bool 		globalLink;	
	bool 		globalMidiClock;
	bool 		shIft = false;
	
	// knob stuff
	float 		k1Local;
	bool 		k1Red = false;
	
	float 		k2Local;
	bool 		k2Red = false;	
	
	float 		k3Local;
	bool 		k3Red = false;
	
	float 		k4Local;
	bool 		k4Red = false;
	
	float 		k5Local;
	bool 		k5Red = false;

};


