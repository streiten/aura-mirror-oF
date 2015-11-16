#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

#ifdef __arm__
    #include <opencv2/opencv.hpp>
    Mat frame,frameProcessed;
#endif

// States: IDLE, Inprogress

// animations shift / scale / rotate / fade / brightness
// parameters: face size / closeup - general movement
// averages... really really slow kenburns... 

//--------------------------------------------------------------
void ofApp::setup(){

    debug = true;
    
    // General
    ofBackground(0, 0, 0);
    //ofSetVerticalSync(true);
    //ofSetFrameRate(120);
    delay = 3000;
    last_time = ofGetElapsedTimeMillis();


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
    
    // img.allocate(10, 10, OF_IMAGE_COLOR);
    
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
    
    // background.setLearningTime(900);
    // background.setThresholdValue(20);
    
    // deprc...
    thresh = 127;
    
    // The Aura Imagery
    ofEnableAlphaBlending();
    dir.listDir("auras/");
    dir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    //ofDisableAntiAliasing();
    //ofDisableSmoothing();
    
    //allocate the vector to have as many ofImages as files
    if( dir.size() ){
        images.assign(dir.size(), ofImage());
    }
    
    // you can now iterate through the files and load them into the ofImage vector
    for(int i = 0; i < (int)dir.size(); i++){
        images[i].loadImage(dir.getPath(i));
    }
    currentImage = 0;
    
    // matrixOverlay.loadImage("radialoveray.png");
    
    ofBackground(ofColor::white);
    sceneSparkle();

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
            // background.update(cam, thresholded);
            // thresholded.update();
        }
    #endif
    
    // generateMirrorFrame();
    
    if( ofGetElapsedTimeMillis() - last_time > delay ) {
        last_time = ofGetElapsedTimeMillis();
        sceneSparkle();
    }
    
    // float hue = fmodf(ofGetElapsedTimef()*10,255);

    sendFrameToMirror();
       
    // img.update();
    generateStripData();

    if (bSendSerialMessage){
        sendCommandToMirror('X');
    }
    
    // unsigned char * pixelsA = myImg.getPixels();

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    #ifdef __arm__
        if(!frame.empty()) {
        //threshold(frame,frameProcessed,thresh,255,0);
        //drawMat(frameProcessed,320,0);
        drawMat(frame,0,0);
        finder.draw();
    }
    #else
        cam.draw(0, 0);
        thresholded.draw(640, 0);
    #endif
    finder.draw();
    
    // ofDrawBitmapString("threshold: " + ofToString(thresh),320,10);
    
    drawMatrix();
    // drawStrip();
    // img.draw(0, 0,100,100);
    
//    if(finder.size() > 0) {
//        if(display_on){
//            sendCommandToMirror('X');
//            display_on = false;
//        }
//    } else {
//        if(!display_on){
//            sendCommandToMirror('X');
//            display_on = true;
//        }
//    }
    
    ofDrawBitmapStringHighlight(ofToString((int) ofGetFrameRate()) + "fps", 10, 20);
    ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 40);

    for(int i = 0; i < finder.size(); i++) {
        ofRectangle object = finder.getObjectSmoothed(i);
        // sunglasses.setAnchorPercent(.5, .5);
        //float scaleAmount = .85 * object.width / sunglasses.getWidth();
        ofPushMatrix();
        // ofTranslate(object.x + object.width / 2., object.y + object.height * .42);
        //ofScale(scaleAmount, scaleAmount);
        //sunglasses.draw(0, 0);
        ofPopMatrix();
        ofPushMatrix();
        ofTranslate(object.getPosition());
        ofDrawBitmapStringHighlight(ofToString(finder.getLabel(i)), 0, 0);
        ofDrawLine(ofVec2f(), toOf(finder.getVelocity(i)) * 10);
        ofPopMatrix();
    }
    
    images[currentImage].draw(ofGetWidth()-100,0,100,100);
    // matrixOverlay.draw(ofGetWidth()-300,0,300,300);

}


void ofApp::sceneSparkle(){
    
    ofColor c;
    // Chose a random pixel and slowly pulse
    
    int randIndexI = (int) ofRandom(10);
    int randIndexJ = (int) ofRandom(10);

    c.set(0, 0, 0);
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            pixelMatrix[i][j] = c;
        }
    }
    c.set(128,128,128);
    pixelMatrix[randIndexI][randIndexJ ] = c;
    
};

void ofApp::sceneMirror(){

};

void ofApp::sceneTransition(){

};


// +++ DISPLAY FUNCTIONS +++


void ofApp::generateStripData() {
    
    int k = 0;
    // Line by line to linear strip
    for(int i = 0; i < 10; i++){
        
        int m = 0;
        for(int j=0; j < 19;j++){
            
            // inverse order on line for every second line in matrix
            int mNew = m;
            if((i+1)%2 == 0) {
                mNew = 9 - m;
            }
            
            if( j%2 == 0 ) {
                pixelStrip[k] = pixelMatrix[i][mNew];
                m++;
            
            //every second pixel is blanks / skipped
            } else {
                ofColor c(0,0,0);
                pixelStrip[k] = c;
            }
            
            k++;
        }
    }
}


void ofApp::drawMatrix() {
    int k = 0;
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 19;j++){
            // Calculate position of Pixels to emulate Hardware zig-zag setup
            int jNew = j;
            // invert from left / right order every second row
            if((i+1)%2 == 0) {
               jNew = 18 - j;
            }
            
            int x = jNew*10+5;
            int y = i*20+5;
            
            ofPushStyle();
            ofFill();
            ofSetColor(pixelStrip[k]);
            ofDrawRectangle(x,y,10,10);
            ofPopStyle();
            
            k++;
            
            if(debug) {
                // ofDrawBitmapString(ofToString(k),x,y);
            }

            
        }
    }
}

void ofApp::drawStrip() {
    for (int i = 0; i<190; i++) {
        ofPushStyle();
        ofSetColor(pixelStrip[i]);
        ofFill();
        ofDrawRectangle(i*4, 0, 4, 4);
        ofPopStyle();
    }
}

void ofApp::generateMirrorFrame() {
    
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            pixelMatrix[i][j] = images[currentImage].getPixelsRef().getColor(i, j);
        }
    }
}


void ofApp::generateMirrorTestFrame() {

    float hue = fmodf(ofGetElapsedTimef()*100,255);
    // float brightness = ofMap(mouseX, 0, 800, 0, 255);
    // cout << brightness << endl;
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            pixelMatrix[i][j].setHsb( hue, ofMap(i, 0, 10, 0, 255), ofMap(j, 10, 0, 0, 128 ) );
            //   img.getPixelsRef().setColor(i, j, pixelMatrix[i][j]);
        }
    }
}

void ofApp::sendFrameToMirror() {
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


// +++ SSH Key Input +++

void ofApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
    keyPressed((int)e.character);
}



//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    ofLogVerbose() << "keyPressed: " << key;
    if(key == 's') { activePiCamSetting = 0 ;}
    if(key == 'S') { activePiCamSetting = 1 ;}
    if(key == 'c') { activePiCamSetting = 2 ;}
    if(key == 'b') { activePiCamSetting = 3 ;}
    if(key == 't') { activePiCamSetting = 4 ;}
    
    if(key == 'i') {
        if (dir.size() > 0){
            currentImage++;
            currentImage %= dir.size();
        }
    }
    
    if(key == ' ') {
        background.reset();
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
        
    #ifdef __arm__
        if( activePiCamSetting == 0 ) { cam.setSaturation(ofMap(x,0,ofGetWidth(),-100,100));}
        if( activePiCamSetting == 1 ) { cam.setSharpness(ofMap(x,0,ofGetWidth(),-100,100));}
        if( activePiCamSetting == 2 ) { cam.setContrast(ofMap(x,0,ofGetWidth(),-100,100));}
        if( activePiCamSetting == 3 ) { cam.setBrightness(ofMap(x,0,ofGetWidth(),0,100));}
        if( activePiCamSetting == 4 ) { thresh = ofMap(x,0,ofGetWidth(),0,255); }
    #endif

    //cam.setISO(value);
    //cam.setVideoStabilisation(value);
    //cam.setExposureCompensation(value);

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
