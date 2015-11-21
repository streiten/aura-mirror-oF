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
    
    ofColor pixelMatrixBlended[10][10];
        
    int currentScene;
    int sceneChange;
    
    void setup();
    void update();
    
    void getSceneBlend(float crossfade, ofColor A[][10], ofColor B[][10]);
    
    // hacks
    bool alwaysOn; 
    
};


#endif /* sceneManager_h */
