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
    // ofSetLogLevel(OF_LOG_ERROR);
    
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
        
//    sparklePulse.animateTo(BRIGHTNESS_MAX);
//    sparklePulse.setRepeatType(LOOP_BACK_AND_FORTH);
//    sparklePulse.setCurve(EASE_IN_EASE_OUT);
    
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
        // Person Watchdog thing
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
        };
//        sparklePulse.reset();
//        sparklePulse.animateTo(BRIGHTNESS_MAX);
    } else {
        personPresentChanged = false;
    }
    
    personPresentLastFrame = personPresent;

    // Updating the animators
//    sparklePulse.update(1.0f/700);
    
    sceneTransitionAnim.update(1.0f/SCENE_FADE);
    
    if(sceneTransitionAnim.hasFinishedAnimating()){
        if(personPresent) {
            //sceneIndex = 1;
        } else {
            //sceneIndex = 0;
        }
    }
    
    // SM.currentScene = 0;
    SM.scenes[SM.currentScene]->update();
    
    if (bSendSerialMessage){
        sendCommandToMirror('X');
    }
    
    // unsigned char * pixels = myImg.getPixels();

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    sendFrameToMirror(SM.scenes[SM.currentScene]->pixelMatrix);

    #ifdef __arm__
        if(!frame.empty()) {
            drawMat(frame,0,0);
        }
    #else
        cam.draw(0, 0);
    #endif
    finder.draw();

    drawLEDMatrix(SM.scenes[SM.currentScene]->pixelMatrix);

    ofDrawRectangle(0, sceneTransitionAnim.val() * ofGetHeight() , ofGetWidth(), 2);
    
    for(int i = 0; i < finder.size(); i++) {
        ofRectangle object = finder.getObjectSmoothed(i);
        String s = "Tracker width:" + ofToString(object.height) + " X: " + ofToString(object.x) + " Y: " +  ofToString(object.y);
        ofDrawBitmapStringHighlight(s , 210, 70);
        // ofDrawBitmapStringHighlight(ofToString(finder.getLabel(i)), 210, 0);
    }
    
        // draw the LED Display & serialized strip simulation
    if(debug){

        SM.mirror.getCurrentImage().draw(ofGetWidth()-100,0,100,100);
        
        ofDrawBitmapStringHighlight(ofToString((int) ofGetFrameRate()) + "fps", 10, 20);
        ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 40);
        
        if(personPresent){
            ofDrawBitmapStringHighlight("Person present",210, 30);
        }
        
        ofDrawBitmapStringHighlight("Calculated Brightness: "+ ofToString(personBrightness), 210, 50);
        ofDrawBitmapStringHighlight(ofToString(sceneTransitionAnim.val()), 210, 90);
        
    }

    gui.draw();

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

//void ofApp::generateMirrorTestFrame() {
//
//    for(int i = 0; i < 10; i++){
//        for(int j=0; j < 10;j++){
//            pixelMatrix[i][j] = ofColor(0,0,0);
//        }
//    }
//
//    pixelMatrix[0][0] = ofColor(255,0,0);
//    pixelMatrix[5][5] = ofColor(0,255,0);
//    pixelMatrix[9][9] = ofColor(0,0,255);
//}

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
    
    paramsGroup1.add(pPiCamBrightness.set("PiCam Brightness", 50, 0, 100));
    paramsGroup1.add(pPiCamContrast.set("PiCam Contrast", 50, 0, 100));
    
    // paramsGroup1.add(pJitterScale[0].set("Jitter scale", true));
    
    gui.add(paramsGroup1);
    
    gui.loadFromFile("settings.xml");
    
}


//--------------------------------------------------------------
void ofApp::pPiCamBrightnessChanged(int & pPiCamBrightness){
#ifdef __arm__
    cam.setBrightness(pPiCamBrightness);
#endif
}

//--------------------------------------------------------------
void ofApp::pPiCamContrastChanged(int & pPiCamContrast){
#ifdef __arm__
    cam.setContrast(pPiCamContrast);
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
        SM.mirror.setImage();
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
//cam.setFlips(value,vflip.get());
//cam.setFlips(hflip.get(),value);
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



