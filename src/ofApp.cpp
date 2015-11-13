#include "ofApp.h"

#ifdef __arm__
    #include <opencv2/opencv.hpp>
    using namespace ofxCv;
    using namespace cv;
    Mat frame,frameProcessed;
#endif

//--------------------------------------------------------------
void ofApp::setup(){

    debug = true;
    
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

    // General
    // ofBackground(0, 0, 0);
    // ofSetVerticalSync(false);
    // ofSetFrameRate(30);
    
    consoleListener.setup(this);
    consoleListener.startThread(false, false);
    
    // img.allocate(10, 10, OF_IMAGE_COLOR);
    #ifdef __arm__
        cam.setup(320,240,false);//setup camera (w,h,color = true,gray = false);
        //cam.setSaturation(value);
        //cam.setSharpness(value);
        //cam.setContrast(value);
        //cam.setBrightness(value);
        //cam.setISO(value);
        //cam.setVideoStabilisation(value);
        //cam.setExposureCompensation(value);
    #endif

    thresh = 127;
}


//--------------------------------------------------------------
void ofApp::update(){
    
    #ifdef __arm__
        frame = cam.grab();
    #endif

    // Fill the Matrix with sample Color
    float hue = fmodf(ofGetElapsedTimef()*100,255);
    // float brightness = ofMap(mouseX, 0, 800, 0, 255);
    // cout << brightness << endl;
    unsigned char serialOutBuffer[301];
    serialOutBuffer[0] = '$';
    int p = 0;

    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
                pixelMatrix[i][j].setHsb( hue, ofMap(i, 0, 10, 0, 255), ofMap(j, 10, 0, 0, 128 ) );
        // Indicate DATA is coming

                // push out the acutal data, r g b pixel per pixel
                serialOutBuffer[3*p+1] = pixelMatrix[i][j].r;
                serialOutBuffer[3*p+2] = pixelMatrix[i][j].g;
                serialOutBuffer[3*p+3] = pixelMatrix[i][j].b;
                p++;

       //   img.getPixelsRef().setColor(i, j, pixelMatrix[i][j]);
            
        }
    }
    
    if(serial.isInitialized()){
        serial.writeBytes(&serialOutBuffer[0],301);
    }
       
   // img.update();
    
    generateStripData();

    if (bSendSerialMessage){
        if(serial.isInitialized()){
            unsigned char cmd[] = {'@','X'};
            serial.writeBytes(&cmd[0],2);
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
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    #ifdef __arm__
    if(!frame.empty()) {
        threshold(frame,frameProcessed,thresh,255,0);
        drawMat(frame,0,0);
        drawMat(frameProcessed,320,0);
    }
    ofDrawBitmapString("threshold: " + ofToString(thresh),320,10);
    #endif

    
    // drawMatrix();
    // Draw the raw strip on the very top for debug
    for (int i = 0; i<190; i++) {
        ofPushStyle();
        ofSetColor(pixelStrip[i]);
        ofPopStyle();
        // ofRectangle(i*4, 0, 4, 4);
    }
    // img.draw(0, 0,100,100);
}


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
            
            int x = jNew*40+20;
            int y = i*80+20;
            
            ofSetColor(pixelStrip[k]);
            k++;
            
            if(debug) {
                ofDrawBitmapString(ofToString(k),x,y);
            }
            
            ofRectangle(x,y,20,20);
            
        }
    }
    
}

void ofApp::onCharacterReceived(SSHKeyListenerEventData& e)
{
    keyPressed((int)e.character);
}



//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    ofLogVerbose() << "keyPressed: " << key;
    if(key == 't' && thresh > 0)   thresh--;
    if(key == 'T' && thresh < 255) thresh++;
    cout << "Keypressed:" << key << endl;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    thresh = ofMap(x,0,ofGetWidth(),0,255);
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


//cam.setSaturation(value);
//cam.setSharpness(value);
//cam.setContrast(value);
//cam.setBrightness(value);
//cam.setISO(value);
//cam.setVideoStabilisation(value);
//cam.setExposureCompensation(value);
//
//exposureMeteringMode.setName(exposureMeteringModes[value]);                            //display the preset name in the UI
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
