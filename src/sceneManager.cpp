//
//  sceneManager.cpp
//  led_matrix
//
//  Created by Alex on 17.11.15.
//
//

#include "ofMain.h"
#include "scenes.h"
#include "sceneManager.h"

void sceneManager::setup(){
    
    currentScene = 0;
    
    intro.setup();
    mirror.setup();
    
    sceneChange = false;
    
    scenes.push_back(&intro);
    scenes.push_back(&mirror);
    
    alwaysOn = true;
    
};

void sceneManager::update(){
    scenes[currentScene]->update();
}

void sceneManager::getSceneBlend(float crossfade, ofColor A[][10], ofColor B[][10]){
    
    for(int i = 0; i < 10; i++){
        for(int j=0; j < 10;j++){
            
            if(alwaysOn) {
                pixelMatrixBlended[i][j] = A[i][j];

            } else {
                pixelMatrixBlended[i][j] = A[i][j].lerp(B[i][j],1 - crossfade);

            }
        }
    }
}
