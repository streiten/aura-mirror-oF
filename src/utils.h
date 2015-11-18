//
//  utils.hpp
//  led_matrix
//
//  Created by Alex on 17.11.15.
//
//

#ifndef utils_h
#define utils_h

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

#endif /* utils_h */
