#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetupOpenGL(640,480,OF_WINDOW);			// <-------- setup the GL context
	ofRunApp(new ofApp());

}
