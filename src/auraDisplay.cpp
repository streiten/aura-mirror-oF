//
//  display.cpp
//  led_matrix
//
//  Created by Alex on 15.11.15.
//
//

#include "auraDisplay.h"

void auraDisplay::setup() {

    // Serial
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 115200;
#ifdef __arm__
    serial.setup("/dev/ttyACM0", baud);
#else
    serial.setup("/dev/tty.usbmodem1216041", baud);
#endif
    memset(bytesReadString, 0, 256);

};

void auraDisplay::drawLEDMatrix(ofColor pixelMatrix[][10]) {
    
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

void auraDisplay::sendFrameToMirror(ofColor pixelMatrix[][10]) {
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

void auraDisplay::sendCommandToMirror(unsigned char cmd) {
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
    }
    
}
