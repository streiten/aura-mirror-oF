#include "ofMain.h"
#include "ofApp.h"


//========================================================================
int main( ){
#ifdef __arm__
    ofSetLogLevel(OF_LOG_ERROR);
#else
    ofSetLogLevel(OF_LOG_VERBOSE);
#endif
    ofSetupOpenGL(640,480,OF_WINDOW);			// <-------- setup the GL context
    ofRunApp(new ofApp());

}
