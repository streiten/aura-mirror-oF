#pragma once

#include "ofMain.h"
// #include "auraDisplay.h"

#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxAnimatableFloat.h"

#include "ConsoleListener.h"
#include "sceneManager.h"
#include "utils.h"

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
    
    bool debug;
    
    // SSH Keystroke Capture
    ConsoleListener consoleListener;
    void onCharacterReceived(SSHKeyListenerEventData& e);
    
    // GUI & Settings
    void setupGui();
    ofxPanel gui;
    ofParameterGroup paramsGroup1;
    ofParameter<float> pColorR;
    ofParameter<float> pColorG;
    ofParameter<float> pColorB;
    ofParameter<int> pMultiplier;
    ofParameter<int> pDivider;
    
    ofParameter<int> pBrightnessMin;
    ofParameter<int> pBrightnessMax;
    ofParameter<int> pBrightness;
    
    ofParameter<int> pPiCamBrightness;
    ofParameter<int> pPiCamContrast;
    
    ofParameter<int>  pPiCamExposureMeteringMode;
    ofParameter<int>  pPiCamExposureCompensation;
    ofParameter<int>  pPiCamExposureMode;
    
    ofParameter<int>  pPiCamISO;
    ofParameter<float> pPiCamRoiX;
    ofParameter<float> pPiCamRoiY;
    ofParameter<float> pPiCamRoiW;
    ofParameter<float> pPiCamRoiH;
    
    void pPiCamBrightnessChanged(int &value);
    void pPiCamContrastChanged(int &value);
    
    void pPiCamRoiXChanged(float &value);
    void pPiCamRoiYChanged(float &value);
    void pPiCamRoiWChanged(float &value);
    void pPiCamRoiHChanged(float &value);
    void pPiCamExposureMeteringModeChanged(int &value);
    void pPiCamExposureModeChanged(int &value);
    void pPiCamExposureCompensationChanged(int &value);
    void pPiCamISOChanged(int &value);
    
    ofRectangle ROI;
    string PiCamExposureModes[14];
    string PiCamExposureMeteringModes[5];
    
    int activeSettingParam;
    
    void generateMirrorTestFrame();
    
    void drawLEDMatrix(ofColor pixelMatrix[][10]);
    void sendFrameToMirror(ofColor pixelMatrix[][10]);
                        
    bool display_on = true;
                        
    
    // SERIAL
    ofSerial    serial;
    
    bool		bSendSerialMessage;			// a flag for sending serial
    char		bytesRead[255];				// data from serial, we will be trying to read 255
    char		bytesReadString[256];			// a string needs a null terminator, so we need 255 + 1 bytes
    int			nBytesRead;					// how much did we read?
    int			nTimesRead;					// how many times did we read?
    
    void sendCommandToMirror(unsigned char cmd);

    // CAM
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
        string PiCam_awbModes[11];
        string PiCam_imageFXLabels[24];
    
    void PiCam_saturationChanged(int &value);
    void PiCam_sharpnessChanged(int &value);
    void PiCam_vstabilisationChanged(bool &value);
    void PiCam_shutterSpeedChanged(int &value);
    void PiCam_rotationChanged(int &value);
    void PiCam_hflipChanged(bool &value);
    void PiCam_vflipChanged(bool &value);
    void PiCam_awbModeChanged(int &value);
    void PiCam_awbGainRChanged(float &value);
    void PiCam_awbGainBChanged(float &value);
    void PiCam_imageFXChanged(int &value);
    #else
        ofVideoGrabber cam;
    #endif
    
    // FACEDETECTION
    ofxCv::ObjectFinder finder;
    ofImage img;
    
    bool personPresent;
    bool personPresentLastFrame;
    bool personPresentChanged;
    
    float personBrightness;
    auraTimer presentTimer;
    
    ofxAnimatableFloat sceneTransitionAnim;
    
    // SCENES
    sceneManager SM;

};



