/*
 * Copyright (c) 2020 Owen Osborn, Critter & Gutiari, Inc.
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include "ofMain.h"
#include "ofApp.h"

int main() {
	ofGLESWindowSettings settings;
	settings.glesVersion=2;
	settings.setSize(1920,1080);
	ofCreateWindow(settings);
    
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());



}
