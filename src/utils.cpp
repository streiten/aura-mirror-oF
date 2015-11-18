//
//  utils.cpp
//  led_matrix
//
//  Created by Alex on 17.11.15.
//
//

#include "utils.h"
#include "ofMain.h"

void auraTimer::set(int millis,bool autoReset){
    this->delay = millis;
    this->autoReset = autoReset;
}

bool auraTimer::check() {
    
    if( (ofGetElapsedTimeMillis() - this->last_time) > this->delay){
        
        if(autoReset){reset();}
        
        return true;
    } else {
        return false;
    }
    
};

int auraTimer::get() {
    return ofGetElapsedTimeMillis() - this->last_time;
}

void auraTimer::reset() {
    this->last_time = ofGetElapsedTimeMillis();
};