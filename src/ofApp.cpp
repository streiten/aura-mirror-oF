#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0, 0, 0);
    debug = true;
    
    ofSetVerticalSync(true);
    bSendSerialMessage = false;
    
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    
    int baud = 9600;
        
    #ifdef __arm__
    serial.setup("/dev/ttyACM0", baud); //open the first device
    #else 
    serial.setup("/dev/tty.usbmodem1216041", baud); //open the first device
    #endif


    memset(bytesReadString, 0, 256);

    ofSetFrameRate(30);
    img.allocate(10, 10, OF_IMAGE_COLOR);

}

//--------------------------------------------------------------
void ofApp::update(){
    
    // Fill the Matrix with sample Color
    float hue = fmodf(ofGetElapsedTimef()*100,255);
    float brightness = ofMap(mouseX, 0, 800, 0, 255);
    // cout << brightness << endl;
    
    // Indicate DATA is coming
    if(serial.isInitialized()){
        serial.writeByte('$');
    }
    
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            pixelMatrix[i][j].setHsb( hue, ofMap(i, 0, 10, 0, 255), ofMap(j, 10, 0, 0, 128 ) );
            
            img.getPixelsRef().setColor(i, j, pixelMatrix[i][j]);
            
            if(serial.isInitialized()){
                // push out the acutal data, r g b pixel per pixel
                serial.writeByte(pixelMatrix[i][j].r);
                serial.writeByte(pixelMatrix[i][j].g);
                serial.writeByte(pixelMatrix[i][j].b);
            }
        }
    }
    
    img.update();
    
    generateStripData();

    if (bSendSerialMessage){
        
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

//--------------------------------------------------------------
void ofApp::draw(){
    
    drawMatrix();
    // Draw the raw strip on the very top for debug
    for (int i = 0; i<190; i++) {
        ofSetColor(pixelStrip[i]);
        ofRect(i*4, 0, 4, 4);
    }
    img.draw(0, 0,100,100);
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
            
            ofRect(x,y,20,20);
            
        }
    }
    
}



//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
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
