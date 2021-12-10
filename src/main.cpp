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
	// this way...
	ofGLESWindowSettings settings;
	settings.glesVersion=2;
	settings.setSize(1920,1080);
	ofCreateWindow(settings);
	ofRunApp(new ofApp());
  

	// or this way... 
//	ofSetupOpenGL(1920,1080, OF_FULLSCREEN);
//	ofRunApp(new ofApp());


}
