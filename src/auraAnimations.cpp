//
//  animations.cpp
//  led_matrix
//
//  Created by Alex on 16.11.15.
//
//

#include "auraAnimations.h"

void auraTimer::set(int millis){
    this->delay = millis;
}

bool auraTimer::check() {

    if( (ofGetElapsedTimeMillis() - this->last_time) > this->delay){
        reset();
        return true;
    } else {
        return false;
    }
    
};

void auraTimer::reset() {
    this->last_time = ofGetElapsedTimeMillis();
};


    

