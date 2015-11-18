#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

#ifdef __arm__
    #include <opencv2/opencv.hpp>
    Mat frame,frameProcessed;
#endif

#define SCENE_FADE 100
#define JITTER_DELAY 1000
#define BRIGHTNESS_MAX 128

// animations shift / scale / rotate / fade / brightness
// parameters: face size / closeup - general movement

//--------------------------------------------------------------
void ofApp::setup(){
    
    debug = true;
    ofSetLogLevel(OF_LOG_ERROR);
    
    // General
    ofBackground(0, 0, 0);
    //ofSetVerticalSync(true);
    //ofSetFrameRate(120);
    
    consoleListener.setup(this);
    consoleListener.startThread(false, false);
    
    // Serial
    bSendSerialMessage = false;
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 115200;
    #ifdef __arm__
        serial.setup("/dev/ttyACM0", baud); //open the first device
    #else 
        serial.setup("/dev/tty.usbmodem1216041", baud); //open the first device
    #endif
    memset(bytesReadString, 0, 256);
    
    // Cam & Facedetection
    finder.setPreset(ObjectFinder::Fast);
    finder.getTracker().setSmoothingRate(0.05);

    #ifdef __arm__
        finder.setup("haarcascade_frontalface_alt2.xml");
        cam.setup(640,480,false);//setup camera (w,h,color = true,gray = false);
        cam.setFlips(false,true);
        cam.setISO(800);
    #else
        cam.videoSettings();
        cam.setup(640,480);
        finder.setup("haarcascade_frontalface_default.xml");
    #endif
    
    presentTimer.set(JITTER_DELAY,false);
    personPresent = false;
    personPresentLastFrame = false;
    personPresentChanged = false;
    personBrightness = BRIGHTNESS_MAX;
    
    
    SM.setup();
        
    // sceneTransitionAnim.setCurve(EASE_IN_EASE_OUT);
    // sceneTransitionAnim.setRepeatType(LOOP_BACK_AND_FORTH_ONCE);
    
    // The Gui
    setupGui();
}



//--------------------------------------------------------------
void ofApp::update(){
    
    #ifdef __arm__
        frame = cam.grab();
        if(!frame.empty()) finder.update(frame);
    #else
        cam.update();
        if(cam.isFrameNew()) {
            finder.update(cam);
        }
    #endif
    
    
    if(finder.size() > 0) {
        // Person present watchdog
        presentTimer.reset();
        personPresent = true;
    }
    
    // prevent factedetection fails to invoke scenechanges
    if(presentTimer.check()) {
        personPresent = false;
    }
    
    if( personPresentLastFrame != personPresent ) {
        cout << "Person present changed!" << endl ;
        personPresentChanged = true;
        if(!sceneTransitionAnim.isAnimating()) {
            sceneTransitionAnim.reset();
            sceneTransitionAnim.animateTo(1);
            
            SM.mirror.setRandomImage();
        
        };
    } else {
        personPresentChanged = false;
    }
    personPresentLastFrame = personPresent;
    
    sceneTransitionAnim.update(1.0f/SCENE_FADE);
    
    if(sceneTransitionAnim.hasFinishedAnimating()){
        if(personPresent) {
            SM.currentScene = 1;
        } else {
            SM.currentScene = 0;
        }
    }
    
    SM.scenes[SM.currentScene]->update();
    
    // unsigned char * pixels = myImg.getPixels();

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    sendFrameToMirror(SM.scenes[SM.currentScene]->pixelMatrix);

    if(debug){
        
#ifdef __arm__
        if(!frame.empty()) {
            drawMat(frame,0,0);
        }
#else
        cam.draw(0, 0);
#endif
        finder.draw();
        
        for(int i = 0; i < finder.size(); i++) {
            ofRectangle object = finder.getObjectSmoothed(i);
            String s = "Tracker width:" + ofToString(object.height) + " X: " + ofToString(object.x) + " Y: " +  ofToString(object.y);
            ofDrawBitmapStringHighlight(s , 210, 70);
        }
        
        ofDrawRectangle(0, sceneTransitionAnim.val() * ofGetHeight() , ofGetWidth(), 2);
        
        drawLEDMatrix(SM.scenes[SM.currentScene]->pixelMatrix);
        if(SM.currentScene == 1) {
            SM.mirror.getCurrentImage().draw(ofGetWidth()-100,0,100,100);
        }
        
        ofDrawBitmapStringHighlight(ofToString((int) ofGetFrameRate()) + "fps", 10, 20);
//        ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 40);
        
        if(personPresent){
            ofDrawBitmapStringHighlight("Person present!",210, 30);
        }
        
//        ofDrawBitmapStringHighlight("Calculated Brightness: "+ ofToString(personBrightness), 210, 50);
        ofDrawBitmapStringHighlight(ofToString(sceneTransitionAnim.val()), 210, 90);
        
        gui.draw();

    }

}


void ofApp::drawLEDMatrix(ofColor pixelMatrix[][10]) {
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            ofPushStyle();
            ofFill();
            ofSetColor(pixelMatrix[i][j]);
            ofDrawRectangle(j * 20 + 10 , i * 20 + 10 ,10,10);
            ofPopStyle();
        }
    }
    
}

void ofApp::sendFrameToMirror(ofColor pixelMatrix[][10]) {
    if(serial.isInitialized()){
        unsigned char serialOutBuffer[301];
        // Indicate DATA is coming
        serialOutBuffer[0] = '$';
        int p = 0;
        for(int i = 0; i < 10; i++){
            for(int j=0; j < 10;j++){
                // push out the acutal data, r g b pixel per pixel
                serialOutBuffer[3*p+1] = pixelMatrix[i][j].r;
                serialOutBuffer[3*p+2] = pixelMatrix[i][j].g;
                serialOutBuffer[3*p+3] = pixelMatrix[i][j].b;
                p++;
            }
        }
        serial.writeBytes(&serialOutBuffer[0],301);
    }
}

void ofApp::sendCommandToMirror(unsigned char cmd) {
    if(serial.isInitialized()){
        
        unsigned char cmdseq[] = {'@', cmd };
        serial.writeBytes(&cmdseq[0],2);
        cout << "Display toggle CMD sent!" << endl;
        
        nTimesRead = 0;
        nBytesRead = 0;
        int nRead  = 0;  // a temp variable to keep count per read
        
        unsigned char bytesReturned[255];
        
        memset(bytesReadString, 0, 256);
        memset(bytesReturned, 0, 255);
        
        while( (nRead = serial.readBytes( bytesReturned, 255)) > 0){
            nTimesRead++;
            nBytesRead = nRead;
        };
        
        memcpy(bytesReadString, bytesReturned, 255);
        cout << "Display said:" << bytesReadString << endl;
        bSendSerialMessage = false;
    }
    
}


void ofApp::setupGui (){
    
    gui.setup("Parameters");
    gui.setPosition(0,200);
    
    paramsGroup1.setName("Settings");
    
    //    paramsGroup1.add(pShow[0].set( "Show/Hide", true ));
    //    paramsGroup1.add(pColorR.set( "R", 128, 0, 255 ));
    //    paramsGroup1.add(pColorG.set( "G", 128, 0, 255 ));
    //    paramsGroup1.add(pColorB.set("B", 128, 0, 255 ));
    //    paramsGroup1.add(pMultiplier.set("Multiply", 1, 1, 50));
    //    paramsGroup1.add(pDivider.set("Divide", 1, 1, 10));
    //    paramsGroup1.add(pBrightness.set("Brightness", 128, 0, 255));
    
    paramsGroup1.add(pBrightnessMin.set("Brightness Min", 140, 0, 300));
    paramsGroup1.add(pBrightnessMax.set("Brightness Max", 250, 140, 300));
    
    pPiCamBrightness.addListener(this, &ofApp::pPiCamBrightnessChanged);
    pPiCamContrast.addListener(this,&ofApp::pPiCamContrastChanged);
    
    paramsGroup1.add(pPiCamBrightness.set("PiCam Brightness", 50, 0, 100));
    paramsGroup1.add(pPiCamContrast.set("PiCam Contrast", 50, 0, 100));

//    paramsGroup1.add(pPiCamISO.set("ISO",300,100,800));
    paramsGroup1.add(pPiCamExposureMeteringMode.set("PiCam Metering Mode", 0,0,4));
    paramsGroup1.add(pPiCamExposureCompensation.set("PiCam Exposure Compensation", 0,-10,10));
    paramsGroup1.add(pPiCamExposureMode.set("PiCam Exposure Mode", 0,0,13));
    paramsGroup1.add(pPiCamRoiX.set("ROI x",0,0,1));
    paramsGroup1.add(pPiCamRoiY.set("ROI y",0,0,1));
    paramsGroup1.add(pPiCamRoiW.set("ROI w",1,0,1));
    paramsGroup1.add(pPiCamRoiH.set("ROI h",1,0,1));
    paramsGroup1.add(pPiCamImageFX.set("image effect",0,0,23));

    
    pPiCamISO.addListener(this,&ofApp::pPiCamISOChanged);
    pPiCamExposureCompensation.addListener(this,&ofApp::pPiCamExposureCompensationChanged);
    pPiCamExposureMeteringMode.addListener(this,&ofApp::pPiCamExposureMeteringModeChanged);
    pPiCamExposureMode.addListener(this,&ofApp::pPiCamExposureModeChanged);
    pPiCamRoiX.addListener(this,&ofApp::pPiCamRoiXChanged);
    pPiCamRoiY.addListener(this,&ofApp::pPiCamRoiYChanged);
    pPiCamRoiW.addListener(this,&ofApp::pPiCamRoiWChanged);
    pPiCamRoiH.addListener(this,&ofApp::pPiCamRoiHChanged);
    pPiCamImageFX.addListener(this,&ofApp::pPiCamImageFXChanged);

    PiCamExposureMeteringModes[0] = "average";
    PiCamExposureMeteringModes[1] = "spot";
    PiCamExposureMeteringModes[2] = "backlit";
    PiCamExposureMeteringModes[3] = "matrix";
    PiCamExposureMeteringModes[4] = "max";
    
    PiCamExposureModes[ 0] = "off";
    PiCamExposureModes[ 1] = "auto";
    PiCamExposureModes[ 2] = "night";
    PiCamExposureModes[ 3] = "night preview";
    PiCamExposureModes[ 4] = "backlight";
    PiCamExposureModes[ 5] = "spotlight";
    PiCamExposureModes[ 6] = "sports";
    PiCamExposureModes[ 7] = "snow";
    PiCamExposureModes[ 8] = "beach";
    PiCamExposureModes[ 9] = "very long";
    PiCamExposureModes[10] = "fixed fps";
    PiCamExposureModes[11] = "antishake";
    PiCamExposureModes[12] = "fireworks";
    PiCamExposureModes[13] = "max";
    
    PiCamImageFXLabels[ 0] = "none";
    PiCamImageFXLabels[ 1] = "negative";
    PiCamImageFXLabels[ 2] = "solarize";
    PiCamImageFXLabels[ 3] = "posterize";
    PiCamImageFXLabels[ 4] = "whiteboard";
    PiCamImageFXLabels[ 5] = "blackboard";
    PiCamImageFXLabels[ 6] = "sketch";
    PiCamImageFXLabels[ 7] = "denoise";
    PiCamImageFXLabels[ 8] = "emboss";
    PiCamImageFXLabels[ 9] = "oilpaint";
    PiCamImageFXLabels[10] = "hatch";
    PiCamImageFXLabels[11] = "gpen";
    PiCamImageFXLabels[12] = "pastel";
    PiCamImageFXLabels[13] = "whatercolour";
    PiCamImageFXLabels[14] = "film";
    PiCamImageFXLabels[15] = "blur";
    PiCamImageFXLabels[16] = "saturation";
    PiCamImageFXLabels[17] = "colour swap";
    PiCamImageFXLabels[18] = "washedout";
    PiCamImageFXLabels[19] = "posterize";
    PiCamImageFXLabels[20] = "colour point";
    PiCamImageFXLabels[21] = "colour balance";
    PiCamImageFXLabels[22] = "cartoon";
    PiCamImageFXLabels[23] = "max";
    
    gui.add(paramsGroup1);
    
    gui.loadFromFile("settings.xml");
    
}


//--------------------------------------------------------------
void ofApp::pPiCamBrightnessChanged(int & pPiCamBrightness){
#ifdef __arm__
    cam.setBrightness(pPiCamBrightness);
#endif
    cout << "pPiCamBrightness Event Handler Bang " << endl;
}

//--------------------------------------------------------------
void ofApp::pPiCamContrastChanged(int & pPiCamContrast){
#ifdef __arm__
    cam.setContrast(pPiCamContrast);
#endif
    cout << "pPiCamContrast Event Handler Bang " << endl;

}


void ofApp::pPiCamISOChanged(int &value){
#ifdef __arm__
    cam.setISO(value);
#endif
cout << "pPiCam Event Handler Bang " << value << endl;

}

void ofApp::pPiCamExposureCompensationChanged(int &value){
#ifdef __arm__
    cam.setExposureCompensation(value);
#endif
    cout << "pPiExposure Compensation Event Handler Bang " << value << endl;

}
void ofApp::pPiCamExposureMeteringModeChanged(int &value){
    pPiCamExposureMeteringMode.setName(PiCamExposureMeteringModes[value]);
#ifdef __arm__
    if(value == pPiCamExposureMeteringMode.getMax()) value = MMAL_PARAM_EXPOSUREMETERINGMODE_MAX;
    cam.setExposureMeteringMode((MMAL_PARAM_EXPOSUREMETERINGMODE_T)value);
#endif
    cout << "pPiExposure Metering Mode Event Handler Bang " << value << endl;


}
void ofApp::pPiCamExposureModeChanged(int &value){
    pPiCamExposureMode.setName(PiCamExposureModes[value]);
#ifdef __arm__
    if(value == pPiCamExposureMode.getMax()) value = MMAL_PARAM_EXPOSUREMODE_MAX;
    cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T)value);
#endif
    cout << "pPiExposure Mode Event Handler Bang " << value << endl;

}
void ofApp::pPiCamRoiXChanged(float &value){
    ROI.x = value;
#ifdef __arm__
    cam.setROI(ROI);
#endif
    cout << "pPiRoiX Event Handler Bang " << value << endl;

}
void ofApp::pPiCamRoiYChanged(float &value){
    ROI.y = value;
#ifdef __arm__
    cam.setROI(ROI);
#endif
    cout << "pPiRoiY Event Handler Bang " << value << endl;

}
void ofApp::pPiCamRoiWChanged(float &value){
    ROI.width = value;
#ifdef __arm__
    cam.setROI(ROI);
#endif
    cout << "pPiRoiW Event Handler Bang " << value << endl;

}
void ofApp::pPiCamRoiHChanged(float &value){
    ROI.height = value;
#ifdef __arm__
    cam.setROI(ROI);
#endif
    cout << "pPiRoiH Event Handler Bang " << value << endl;

}


void ofApp::pPiCamImageFXChanged(int &value){
    pPiCamImageFX.setName(PiCamImageFXLabels[value]);//display the preset name in the UI
#ifdef __arm__
    if(value == imageFX.getMax()) value = MMAL_PARAM_IMAGEFX_MAX;//the preset max value is different from the UI
    cam.setImageFX((MMAL_PARAM_IMAGEFX_T)value);
#endif
    
}


// +++ SSH Key Input +++
void ofApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
    keyPressed((int)e.character);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    // Brightness Min & Max
    if(key == 'q') { activeSettingParam = 1 ;}
    if(key == 'w') { activeSettingParam = 2 ;}
    
    // PiCam contrast & brightness
    if(key == 'b') { activeSettingParam = 3 ;}
    if(key == 'c') { activeSettingParam = 4 ;}
    
    // Save and Load Settings
    if(key == 's'){
        gui.saveToFile("settings.xml");
        cout << "Settings saved" << endl;
    }
    if(key == 'l'){
        gui.loadFromFile("settings.xml");
        cout << "Settings loaded" << endl;
    }
    
    if(key == 'o') {
        SM.currentScene++;
        SM.currentScene %= 2;
        cout << "Current Scene: " << SM.currentScene << endl;
    }
    
    if(key == 'i') {
        SM.mirror.setRandomImage();
    }
    
    if(key == '+') {
        
        if(activeSettingParam == 1) {
            if(pBrightnessMin < 300) {
                pBrightnessMin += 5;
                cout << "pBrightnessMin: " << pBrightnessMin << endl;
            }
        }
        if(activeSettingParam == 2) {
            if(pBrightnessMax < 300) {
                pBrightnessMax += 5;
                cout << "pBrightnessMax: " << pBrightnessMax << endl;
            }
        }
        
        if(activeSettingParam == 3) {
            if(pPiCamBrightness < 100) {
                pPiCamBrightness += 5;
                cout << "pPiCamBrightness: " << pPiCamBrightness << endl;
            }
        }
        if(activeSettingParam == 4) {
            if(pPiCamContrast < 100) {
                pPiCamContrast += 5;
                cout << "pPiCamContrast: " << pPiCamContrast << endl;
            }
        }
    }
    
    if(key == '-') {
        
        if(activeSettingParam == 1) {
            if(pBrightnessMin > 140) {
                pBrightnessMin -= 5;
                cout << "pBrightnessMin: " << pBrightnessMin << endl;
                
            }
        }
        if(activeSettingParam == 2) {
            if(pBrightnessMax > 140) {
                pBrightnessMax -= 5;
                cout << "pBrightnessMax: " << pBrightnessMax << endl;
                
            }
        }

        if(activeSettingParam == 3) {
            if(pPiCamBrightness > 0) {
                pPiCamBrightness -= 5;
                cout << "pPiCamBrightness: " << pPiCamBrightness << endl;

            }
        }
        if(activeSettingParam == 4) {
            if(pPiCamContrast > 0) {
                pPiCamContrast -= 5;
                cout << "pPiCamContrast: " << pPiCamContrast << endl;

            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    bSendSerialMessage = true;
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}


//exposureMeteringMode.setName(exposureMeteringModes[value]);
//display the preset name in the UI
//if(value == exposureMeteringMode.getMax()) value = MMAL_PARAM_EXPOSUREMETERINGMODE_MAX;//the preset max value is different from the UI
//cam.setExposureMeteringMode((MMAL_PARAM_EXPOSUREMETERINGMODE_T)value);
//
//exposureMode.setName(exposureModes[value]);//display the preset name in the UI
//if(value == exposureMode.getMax()) value = MMAL_PARAM_EXPOSUREMODE_MAX;//the preset max value is different from the UI
//cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T)value);
//
//awbMode.setName(awbModes[value]);//display the preset name in the UI
//if(value == awbMode.getMax()) value = MMAL_PARAM_AWBMODE_MAX;//the preset max value is different from the UI
//cam.setAWBMode((MMAL_PARAM_AWBMODE_T)value);
//
//imageFX.setName(imageFXLabels[value]);//display the preset name in the UI
//if(value == imageFX.getMax()) value = MMAL_PARAM_IMAGEFX_MAX;//the preset max value is different from the UI
//cam.setImageFX((MMAL_PARAM_IMAGEFX_T)value);
//
//cam.setShutterSpeed(value);
//cam.setRotation(value);
//ROI.x = value;
//cam.setROI(ROI);
//ROI.y = value;
//cam.setROI(ROI);
//ROI.width = value;
//cam.setROI(ROI);
//ROI.height = value;
//cam.setROI(ROI);
//cam.setAWBGains(value,awbGainB.get());
//cam.setAWBGains(awbGainR.get(),value);



