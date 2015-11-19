//
//  animations.cpp
//  led_matrix
//
//  Created by Alex on 16.11.15.
//
//

#include "scenes.h"
#include "ofxAnimatable.h"

#define BRIGHTNESS_MAX 128

#ifdef __arm__
#define FPS 10
#else
#define FPS 60
#endif


void scene::setup() {};
void scene::update() {};

void scene::setFrameBrightness( float brightness ) {
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            pixelMatrix[i][j].setBrightness( brightness );
        }
    }
}

void sceneIntro::setup(){
    
    pulse.animateTo(BRIGHTNESS_MAX);
    pulse.setRepeatType(LOOP_BACK_AND_FORTH);
    pulse.setCurve(EASE_IN_EASE_OUT);
    pulse.setDuration(10);
    
};

void sceneIntro::update(){
    
    pulse.update(1.0f / FPS);
    
    int k = 0;
    int setMatrix[] = {
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,0,0,0,0,
        0,0,0,0,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0
    };
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            if(setMatrix[k] == 0 ){
                pixelMatrix[i][j] = ofColor(0,0,0);
            } else {
                pixelMatrix[i][j] = ofColor(255,255,255);
                pixelMatrix[i][j].setBrightness(pulse.val());
            };
            k++;
        }
    }
};


void sceneMirror::setup(){
    
    pulse.animateTo(BRIGHTNESS_MAX);
    pulse.setRepeatType(LOOP_BACK_AND_FORTH);
    pulse.setCurve(EASE_IN_EASE_OUT);
    pulse.setDuration(10);

    shiftTimer.set(500,true);

    // ofEnableAlphaBlending();
    // ofDisableAntiAliasing();
    // ofDisableSmoothing();
    
    dir.listDir("auras/");
    dir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    
    //allocate the vector to have as many ofImages as files
    if( dir.size() ){
        images.assign(dir.size(), ofImage());
    }
    
    // img.allocate(10, 10, OF_IMAGE_COLOR);
    for(int i = 0; i < (int)dir.size(); i++){
        images[i].loadImage(dir.getPath(i));
    }
    
    setRandomImage();

};

void sceneMirror::update(){
    
    pulse.update(1.0f / FPS);
    
    int k = 0;
    int setMatrix[] = {
        0,0,0,1,1,1,1,0,0,0,
        0,0,1,1,1,1,1,1,0,0,
        0,1,1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,
        0,1,1,1,1,1,1,1,1,0,
        0,0,1,1,1,1,1,1,0,0,
        0,0,0,1,1,1,1,0,0,0
    };
    
    generateMatrixFromImage();
    
//    if( shiftTimer.check() ) {
//        
//        for(int i = 0; i < shiftIndex ;i++) {
//            shiftMatrix(pixelMatrix,1);
//        }
//        shiftIndex++;
//        if(shiftIndex > 9) shiftIndex = 0;
//    }
    
//    for(int i = 0; i < finder.size(); i++) {
//        ofRectangle person = finder.getObjectSmoothed(i);
//        personBrightness = ofMap(person.width, pBrightnessMin, pBrightnessMax, 0,BRIGHTNESS_MAX,true);
//    }
//    
    setFrameBrightness(pulse.val());
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            if(setMatrix[k] == 0 ){
                pixelMatrix[i][j] = ofColor(0,0,0);
            }
            k++;
        }
    }
    
};

void sceneMirror::shiftMatrix(int dir) {
    ofColor line_buffer[10];
    switch(dir) {
        case 0:
            memcpy(&line_buffer,&pixelMatrix, sizeof(ofColor)*10);
            for(int i=0;i<9;i++) {
                memcpy(&pixelMatrix[i],&pixelMatrix[i+1], sizeof(ofColor)*10);
            }
            memcpy(&pixelMatrix[9], &line_buffer, sizeof(ofColor)*10);
            break;
            
        case 1:
            memcpy(&line_buffer,&pixelMatrix[9], sizeof(ofColor)*10);
            for(int i=0;i<9;i++) {
                memcpy(&pixelMatrix[9-i],&pixelMatrix[8-i], sizeof(ofColor)*10);
            }
            memcpy(&pixelMatrix[0], &line_buffer, sizeof(ofColor)*10);
            break;
            
        default:
            break;
    }
}

void sceneMirror::setRandomImage(){
    if (dir.size() > 0){
        currentImage = ofRandom(dir.size());
        cout << "Image Index: " << currentImage << endl;
    }
};

ofImage sceneMirror::getCurrentImage(){
    return images[currentImage];
};

void sceneMirror::generateMatrixFromImage(){
    ofColor c;
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            c = images[currentImage].getPixelsRef().getColor(j,i);
            pixelMatrix[i][j] = c;
        }
    }
};



