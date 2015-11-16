//
//  animations.hpp
//  led_matrix
//
//  Created by Alex on 16.11.15.
//
//

#ifndef auraAnimations_h
#define auraAnimations_h

#include "ofMain.h"

class auraTimer {
    
public:
    
    int delay;
    int last_time;
    bool autoReset;
    
    void set(int millis, bool autoReset);
    int get();
    bool check();
    void reset();
    
};

#endif /* animations_h */
