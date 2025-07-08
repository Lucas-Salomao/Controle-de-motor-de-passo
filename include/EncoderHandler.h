#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H

#include <Arduino.h>
#include "config.h"

class EncoderHandler {
private:
    int lastClkState;
    int direction;
    bool buttonPressed;
    bool lastButtonState;
    unsigned long lastDebounceTime;
    static const unsigned long debounceDelay = 50;
    int pulseCounter;
    
public:
    EncoderHandler();
    void begin();
    void update();
    int getDirection();
    bool isPressed();
    void resetDirection();
};

#endif