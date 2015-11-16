#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

    ofSetLogLevel(OF_LOG_VERBOSE);
    #ifdef __arm__
        ofSetupOpenGL(640,480,OF_WINDOW);			// <-------- setup the GL context
    #else
        ofSetupOpenGL(640,480,OF_WINDOW);			// <-------- setup the GL context
    #endif
	
    ofRunApp(new ofApp());

}
