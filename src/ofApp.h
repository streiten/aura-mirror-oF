#pragma once

#include "ofMain.h"
#include "ConsoleListener.h"

#ifdef __arm__
    #include "ofxCvPiCam.h"
#endif



class ofApp : public ofBaseApp , public SSHKeyListener{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void drawMatrix();
    ofColor pixelMatrix[10][10];
    ofColor pixelStrip[200];
    
    void generateStripData();
    
    bool debug;
    int matrixStyle;
    
    bool		bSendSerialMessage;			// a flag for sending serial
    char		bytesRead[255];				// data from serial, we will be trying to read 255
    char		bytesReadString[256];			// a string needs a null terminator, so we need 255 + 1 bytes
    int			nBytesRead;					// how much did we read?
    int			nTimesRead;					// how many times did we read?
    
    ofSerial	serial;
    
    ofImage img;
    
    int thresh;
    
#ifdef __arm__
    ofxCvPiCam cam;
#endif
    
    ConsoleListener consoleListener;
    void onCharacterReceived(SSHKeyListenerEventData& e);

    
};
