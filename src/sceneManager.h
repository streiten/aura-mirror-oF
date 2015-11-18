//
//  sceneManager.hpp
//  led_matrix
//
//  Created by Alex on 17.11.15.
//
//

#ifndef sceneManager_h
#define sceneManager_h
#include "scenes.h"

class sceneManager : public ofBaseApp{
    
public:
    
    vector < scene * > scenes;
    
    sceneIntro intro;
    sceneMirror mirror;
    
    int currentScene;
    void setup();
    void update();
};


#endif /* sceneManager_h */
