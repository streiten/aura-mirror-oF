//
//  scenes.h
//  led_matrix
//
//  Created by Alex on 16.11.15.
//
//

#ifndef scenes_h
#define scenes_h

#include "ofMain.h"
#include "utils.h"
#include "ofxAnimatableFloat.h"

class scene {

    ofxAnimatableFloat fader;

public:
    
    ofColor pixelMatrix[10][10];
    
    //void start();
    //void stop();
    
    virtual void setup();
    virtual void update();
    
    // ofColor getPixelMatrix();
    //bool isChangeing();
    //bool isRunning();
    
    void setBrightness(float brightness);
    
    scene(){};
    ~scene(){}
    
};




class sceneIntro : public scene {
    
    auraTimer sparkleTimer;

public:

    void setup();
    void update();
    
    // void setBrightness(float brightness);

};


class sceneMirror : public scene {
    
    auraTimer shiftTimer;
    int shiftIndex;

    ofDirectory dir;
    vector<ofImage> images;

public:
    int currentImage;
    
    void setup();
    void update();
    void setImage();
    ofImage getCurrentImage();
    
protected:
    void shiftMatrix(int dir);
    void generateMatrixFromImage();
    // void setBrightness(float brightness);
};


#endif /* animations_h */
