#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxAnimatableFloat.h"

#include "ConsoleListener.h"
#include "auraAnimations.h"
// #include "auraDisplay.h"


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
    void drawStrip();
    
    ofColor pixelMatrix[10][10];
    ofColor pixelStrip[200];
    void generateStripData();

    void generateMirrorTestFrame();
    void generateMirrorFrame();
    void setMirrorFrameBrightness(float brightness);

    void sendFrameToMirror();
    bool display_on = true;
    int matrixStyle;
    
    bool debug;
    
    ofSerial    serial;
    
    bool		bSendSerialMessage;			// a flag for sending serial
    char		bytesRead[255];				// data from serial, we will be trying to read 255
    char		bytesReadString[256];			// a string needs a null terminator, so we need 255 + 1 bytes
    int			nBytesRead;					// how much did we read?
    int			nTimesRead;					// how many times did we read?
    
    ofImage img;
    int thresh;
    int activePiCamSetting;

    #ifdef __arm__
        ofxCvPiCam cam;
        int PiCam_saturation;
        int PiCam_sharpness;
        int PiCam_contrast;
        int PiCam_brightness;
        int PiCam_ISO;
        bool PiCam_vstabilisation;
        int PiCam_exposureMeteringMode;
        int PiCam_exposureCompensation;
        int PiCam_exposureMode;
        int PiCam_shutterSpeed;
        int PiCam_rotation;
        bool PiCam_hflip;
        bool PiCam_vflip;
        float PiCam_roiX;
        float PiCam_roiY;
        float PiCam_roiW;
        float PiCam_roiH;
        int PiCam_awbMode;
        float PiCam_awbGainR;
        float PiCam_awbGainB;
        int PiCam_imageFX;
        string exposureModes[14];
        string exposureMeteringModes[5];
        string awbModes[11];
        string imageFXLabels[24];
    #else
        ofVideoGrabber cam;
    #endif
    
    // Facedetection & Background substraction
    ofxCv::ObjectFinder finder;
    ofxCv::RunningBackground background;
    ofImage thresholded;

    ConsoleListener consoleListener;
    void onCharacterReceived(SSHKeyListenerEventData& e);
    
    void sendCommandToMirror(unsigned char cmd);
    
    // the arua images 
    ofDirectory dir;
    vector<ofImage> images;
    // ofImage matrixOverlay;
    
    int currentImage;
    
    int shiftIndex;
    void shiftMatrix(int dir);
    
    auraTimer sparkleTimer;
    auraTimer shiftTimer;
    auraTimer presentTimer;
    bool personPresent;
    float personBrightness;
    
    void sceneSparkle();
    void sceneMirror();
    void sceneTransition();
    
    // ofColor pixelMatrix;
    int sceneIndex;
    int sceneCount;
    
    // GUI
    void setupGui();
    ofxPanel gui;
    ofParameterGroup paramsGroup1;
    ofParameter<float> pColorR;
    ofParameter<float> pColorG;
    ofParameter<float> pColorB;
    ofParameter<int> pMultiplier;
    ofParameter<int> pDivider;
    ofParameter<int> pBrightness;
    
    ofxAnimatableFloat shift;
    ofxAnimatableFloat sparklePulse;

};
