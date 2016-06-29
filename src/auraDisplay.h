//
//  display.hpp
//  led_matrix
//
//  Created by Alex on 15.11.15.
//
//

#include "ofMain.h"

#ifndef auraDisplay_h
#define auraDisplay_h

class auraDisplay {

public:
    
    void setup();
    void update();
    
    
    void generateMirrorTestFrame();
    void drawLEDMatrix(ofColor pixelMatrix[][10]);
    void sendFrameToMirror(ofColor pixelMatrix[][10]);
    
    bool display_on = true;
    
    // SERIAL
    ofSerial    serial;
    char		bytesRead[255];				// data from serial, we will be trying to read 255
    char		bytesReadString[256];			// a string needs a null terminator, so we need 255 + 1 bytes
    int			nBytesRead;					// how much did we read?
    int			nTimesRead;					// how many times did we read?
    
    void sendCommandToMirror(unsigned char cmd);
    
};

#endif /* display_hpp */

