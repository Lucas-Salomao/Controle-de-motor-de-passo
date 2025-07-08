#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    
public:
    DisplayManager();
    void begin();
    void clear();
    // void showMainMenu(int selectedIndex = 0);
    // MODIFICADO: Assinatura da função para suportar a lista de itens e a rolagem
    void showMainMenu(const char* items[], int totalItems, int selectedIndex, int startIndex);
    void showCycleProgress(int currentStep, int totalSteps);
    void showCycleComplete();
    void showAngleSetup(int angle);
    void showPositioning(int targetAngle, int stepsToMove);
    void showMotorDisabled();
     void showMicrostepSetup(int selectedIndex);
    void showError(const char* message);
    Adafruit_SSD1306* getDisplay();
};

#endif