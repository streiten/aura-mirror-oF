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
    
    scenes.push_back(&intro);
    scenes.push_back(&mirror);
    
};

void sceneManager::update(){
    scenes[currentScene]->update();
}
