#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

#ifdef __arm__
    #include <opencv2/opencv.hpp>
    Mat frame,frameProcessed;
#endif

#define PRESENT_DELAY 2000
#define BRIGHTNESS_MAX 128

#ifdef __arm__
#define FPS 10
#else
#define FPS 60
#endif

// animations shift / scale / rotate / fade / brightness
// parameters: face size / closeup - general movement

//--------------------------------------------------------------
void ofApp::setup(){
    
    // General
    ofBackground(0, 0, 0);
    ofSetFrameRate(60);
    
    consoleListener.setup(this);
    consoleListener.startThread(false, false);
    
    fpsOutTimer.set(1000,true);
    
    ledDisplay.setup();
    
    // Cam & Facedetection
    finder.setPreset(ObjectFinder::Fast);
    finder.setFindBiggestObject(true);
    finder.getTracker().setSmoothingRate(0.3);
    

    #ifdef __arm__
        finder.setup("haarcascade_frontalface_alt2.xml");
        cam.setup(640,480,false);
        cam.setFlips(false,true);
        debug = false;
    #else
        cam.videoSettings();
        cam.setup(640,480);
        finder.setup("haarcascade_frontalface_default.xml");
        debug = true;
    #endif
    
    presentTimer.set(PRESENT_DELAY,false);
    personPresent = false;
    personPresentLastFrame = false;
    personPresentChanged = false;
    
    aFactor = 0;
    aFactorLast = 0;
    
    // sceneBlend.animateTo(0);
    sceneBlend.setCurve(EASE_IN_EASE_OUT);
    sceneBlend.setDuration(5);
    
    SM.setup();
    
    sawSomeone = false;
    
    // the Gui
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
        
//        ofRectangle object = finder.getObjectSmoothed(0);
        
//        aFactor = aFactorLast - object.width;
//        ofDrawBitmapStringHighlight(ofToString(aFactor), 210, 120);
//        aFactorLast = object.width;
    }
    
    // prevent factedetection fails to be treated as person leaving
    if(presentTimer.check()) {
        personPresent = false;
    }
    
    if(fpsOutTimer.check()) {
        cout << "FPS:" << ofGetFrameRate() << endl;
    }
    
    sceneBlend.update( 1.0f / FPS );
    
    // Person present
    if( personPresentLastFrame != personPresent ) {
        cout << "Person present changed!" << endl ;
        personPresentChanged = true;
        
        // person found
        if(personPresent){
            sceneBlend.animateTo(1);
        // person lost
        } else {
            sceneBlend.animateTo(0);
        }
        
    } else {
        personPresentChanged = false;
    }
    personPresentLastFrame = personPresent;

    // State changes
        // just came to idle mode
        if((sceneBlend.val() == 0)) {
            
            if(SM.sceneChange)
            {
                SM.sceneChange = false;
                
		if(!SM.alwaysOn){
		SM.mirror.setRandomImage();
                SM.mirror.generateMatrixFromImage();
}

                cout << "Scene Idle entered!" << endl;
            }
        
        // just came to mirror mode
        } else if ((sceneBlend.val() == 1) ) {
           
            if(SM.sceneChange)
            {
                SM.sceneChange = false;
                sawSomeone = true;
                cout << "Scene Mirror entered!" << endl;
            }

        } else {
            SM.sceneChange = true;
        }

    SM.scenes[0]->update();
    
    if(sawSomeone){
        SM.scenes[1]->update();
    }
    
    SM.getSceneBlend((float) sceneBlend.val() ,SM.mirror.pixelMatrix, SM.intro.pixelMatrix);

    // unsigned char * pixels = myImg.getPixels();

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ledDisplay.sendFrameToMirror(SM.pixelMatrixBlended);

    if(debug){
        ofDrawBitmapStringHighlight(ofToString((int) ofGetFrameRate()) + "fps", 10, 20);

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
            // ofDrawBitmapStringHighlight(ofToString(finder.getLabel(0)), 210, 100);
        }
        
        ofDrawRectangle(0, sceneBlend.val() * ofGetHeight() , ofGetWidth(), 2);
        
        ledDisplay.drawLEDMatrix(SM.pixelMatrixBlended);
        
        SM.mirror.getCurrentImage().draw(ofGetWidth()-100,0,100,100);
        
        if(personPresent){
            ofDrawBitmapStringHighlight("Person present!",210, 30);
        }
        
//      ofDrawBitmapStringHighlight("Aural Factor:" + ofToString(aFactor), 210, 50);
        //ofDrawBitmapStringHighlight(ofToString(sceneBlend.val()), 210, 90);

        gui.draw();
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
    
    paramsGroup1.add(pBrightness.set("Brightness", 128, 10, 255));
    
    pBrightness.addListener(this, &ofApp::pBrightnessChanged);

//    paramsGroup1.add(pBrightnessMin.set("Brightness Min", 140, 0, 300));
//    paramsGroup1.add(pBrightnessMax.set("Brightness Max", 250, 140, 300));
    
    pPiCamBrightness.addListener(this, &ofApp::pPiCamBrightnessChanged);
    pPiCamContrast.addListener(this,&ofApp::pPiCamContrastChanged);
    
    paramsGroup1.add(pPiCamBrightness.set("PiCam Brightness", 50, 0, 100));
    paramsGroup1.add(pPiCamContrast.set("PiCam Contrast", 50, 0, 100));

    paramsGroup1.add(pPiCamISO.set("ISO",300,100,800));
    paramsGroup1.add(pPiCamExposureMeteringMode.set("PiCam Metering Mode", 0,0,4));
    paramsGroup1.add(pPiCamExposureCompensation.set("PiCam Exposure Compensation", 0,-10,10));
    paramsGroup1.add(pPiCamExposureMode.set("PiCam Exposure Mode", 0,0,13));
    paramsGroup1.add(pPiCamRoiX.set("ROI x",0,0,1));
    paramsGroup1.add(pPiCamRoiY.set("ROI y",0,0,1));
    paramsGroup1.add(pPiCamRoiW.set("ROI w",1,0,1));
    paramsGroup1.add(pPiCamRoiH.set("ROI h",1,0,1));
//    paramsGroup1.add(pPiCamImageFX.set("image effect",0,0,23));
    
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
void ofApp::pBrightnessChanged(int &value){
    SM.globalBrightness = value;
    SM.mirror.pulse.animateFromTo(value-10,value);
    SM.intro.pulse.animateTo(value);

    cout << "pBrightness Event Handler Bang: " << value  << endl;
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
    if(value == pPiCamImageFX.getMax()) value = MMAL_PARAM_IMAGEFX_MAX;//the preset max value is different from the UI
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
    

    if(key == '*') {
        if(SM.alwaysOn) {
            SM.alwaysOn = false;
            cout << "Always on OFF now!" << endl;
        } else {
            SM.alwaysOn = true;
            cout << "Always on ON now!" << endl;
        }
    }
    
    if(key == 'd') {
        if(debug) {
            debug = false;
        } else {
            debug = true;
        }
        cout << "Toggle Debug!" << endl;
    }
    
    
    if(key == 'q') {
        activeSettingParam = 1;
        cout << "Setting now" << endl;
    }
    if(key == 'w') {
        activeSettingParam = 2;
        cout << "Setting now" << endl;
    }
    if(key == 'b') {
        activeSettingParam = 3;
        cout << "Setting brightness now" << endl;
    }
    if(key == 'c') {
        activeSettingParam = 4;
        cout << "Setting contrast now" << endl;
    }
    if(key == 'x') {
        activeSettingParam = 5;
        cout << "Setting X now" << endl;
    }
    if(key == 'y') {
        activeSettingParam = 6;
        cout << "Setting Y now" << endl;
    }
    if(key == 'r') {
        activeSettingParam = 7;
        cout << "Setting ROI Width now" << endl;
    }
    if(key == 'R') {
        activeSettingParam = 8;
        cout << "Setting ROI Height now" << endl;
    }
    if(key == 'B') {
        activeSettingParam = 9;
        cout << "Setting Global Brightness now" << endl;
    }
    
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
        if(activeSettingParam == 5) {
            if(pPiCamRoiX < 100) {
                pPiCamRoiX += 0.1;
                cout << "pPiCamRoiX: " << pPiCamRoiX << endl;
            }
        }
        if(activeSettingParam == 6) {
            if(pPiCamRoiY < 100) {
                pPiCamRoiY += 0.1;
                cout << "pPiCamRoiY: " << pPiCamRoiY << endl;
            }
        }
        
        if(activeSettingParam == 7) {
            if(pPiCamRoiW < 100) {
                pPiCamRoiW += 0.1;
                cout << "pPiCamRoiW: " << pPiCamRoiW << endl;
            }
        }
        if(activeSettingParam == 8) {
            if(pPiCamRoiH < 100) {
                pPiCamRoiH += 0.1;
                cout << "pPiCamRoiH: " << pPiCamRoiH << endl;
            }
        }
        if(activeSettingParam == 9) {
            if(pBrightness < 255) {
                pBrightness += 5;
                cout << "pBrightness: " << pBrightness << endl;
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
        if(activeSettingParam == 5) {
            if(pPiCamRoiX < 100) {
                pPiCamRoiX -= 0.1;
                cout << "pPiCamRoiX: " << pPiCamRoiX << endl;
            }
        }
        if(activeSettingParam == 6) {
            if(pPiCamRoiY < 100) {
                pPiCamRoiY -= 0.1;
                cout << "pPiCamRoiY: " << pPiCamRoiY << endl;
            }
        }
        
        if(activeSettingParam == 7) {
            if(pPiCamRoiW < 100) {
                pPiCamRoiW -= 0.1;
                cout << "pPiCamRoiW: " << pPiCamRoiW << endl;
            }
        }
        if(activeSettingParam == 8) {
            if(pPiCamRoiH < 100) {
                pPiCamRoiH -= 0.1;
                cout << "pPiCamRoiH: " << pPiCamRoiH << endl;
            }
        }
        if(activeSettingParam == 9) {
            if(pBrightness > 10) {
                pBrightness -= 5;
                cout << "pBrightness: " << pBrightness << endl;
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



