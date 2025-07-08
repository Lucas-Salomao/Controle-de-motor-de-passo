#ifndef STEPPER_CONTROLLER_H
#define STEPPER_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

class StepperController {
private:
    bool enabled;
    int currentDirection; // 1 = horário, -1 = anti-horário
    
public:
    StepperController();
    void begin();
    void enable();
    void disable();
    void moveOneStep(bool clockwise = true);
    void moveSteps(int steps);
    void setDirection(bool clockwise);
    bool isEnabled();
};

#endif