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
    
    ofxAnimatableFloat pulse;
    ofColor pixelMatrix[10][10];
    
    virtual void setup();
    virtual void update();
    
    
    //void start();
    //void stop();
    //bool isChangeing();
    //bool isRunning();
    
    void setFrameBrightness(float brightness);
    
    scene(){};
    ~scene(){}
    
};




class sceneIntro : public scene {
    
public:

    void setup();
    void update();

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
    void setRandomImage();
    ofImage getCurrentImage();
    
protected:
    void shiftMatrix(int dir);
    void generateMatrixFromImage();
};


#endif /* animations_h */
