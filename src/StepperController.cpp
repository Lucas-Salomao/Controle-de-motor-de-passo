#include "StepperController.h"

StepperController::StepperController() {
    enabled = false;
    currentDirection = 1;
}

void StepperController::begin() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);
    digitalWrite(ENABLE_PIN, HIGH); // HIGH = desabilitado na maioria dos drivers
    
    enabled = false;
    
    Serial.println("StepperController inicializado");
}

void StepperController::enable() {
    digitalWrite(ENABLE_PIN, LOW); // LOW = habilitado
    enabled = true;
    Serial.println("Motor de passo habilitado");
}

void StepperController::disable() {
    digitalWrite(ENABLE_PIN, HIGH); // HIGH = desabilitado
    enabled = false;
    Serial.println("Motor de passo desabilitado");
}

void StepperController::setDirection(bool clockwise) {
    currentDirection = clockwise ? 1 : -1;
    digitalWrite(DIR_PIN, clockwise ? LOW : HIGH);
    delayMicroseconds(10); // Pequeno delay para estabilizar sinal de direção
}

void StepperController::moveOneStep(bool clockwise) {
    if (!enabled) return;
    
    setDirection(clockwise);
    
    // Gera pulso de step
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_DELAY_US / 2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_DELAY_US / 2);
    
    Serial.printf("Step executado - direção: %s\n", clockwise ? "horário" : "anti-horário");
}

void StepperController::moveSteps(int steps) {
    if (!enabled || steps == 0) return;
    
    bool clockwise = steps > 0;
    int absSteps = abs(steps);
    
    setDirection(clockwise);
    
    Serial.printf("Movendo %d steps - direção: %s\n", absSteps, clockwise ? "horário" : "anti-horário");
    
    for (int i = 0; i < absSteps; i++) {
        // Gera pulso de step
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(STEP_DELAY_US / 2);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(STEP_DELAY_US / 2);
        
        // Pequeno delay entre steps para suavidade
        delay(5);
    }
    
    Serial.printf("Movimento de %d steps concluído\n", absSteps);
}

bool StepperController::isEnabled() {
    return enabled;
}