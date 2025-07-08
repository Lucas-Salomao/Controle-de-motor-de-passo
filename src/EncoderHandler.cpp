#include "EncoderHandler.h"

EncoderHandler::EncoderHandler() {
    direction = 0;
    buttonPressed = false;
    lastButtonState = HIGH;
    lastDebounceTime = 0;
}

void EncoderHandler::begin() {
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    
    lastClkState = digitalRead(ENCODER_CLK);
    
    Serial.println("EncoderHandler inicializado");
}

void EncoderHandler::update() {
    // --- Lógica de Rotação (Inalterada) ---
    int currentClkState = digitalRead(ENCODER_CLK);
    
    if (currentClkState != lastClkState) {
        // Incrementa ou decrementa o contador de pulsos a cada pulso detectado
        if (digitalRead(ENCODER_DT) != currentClkState) {
            pulseCounter++; // Sentido horário
        } else {
            pulseCounter--; // Sentido anti-horário
        }

        // Verifica se o contador atingiu o limiar definido em config.h
        if (abs(pulseCounter) >= ENCODER_PULSES_PER_STEP) {
            // Se atingiu, define a direção do passo e zera o contador
            if (pulseCounter > 0) {
                direction = 1;
                Serial.println("Encoder: PASSO HORÁRIO");
            } else {
                direction = -1;
                Serial.println("Encoder: PASSO ANTI-HORÁRIO");
            }
            pulseCounter = 0; // Zera o contador para o próximo passo
        }
        
        lastClkState = currentClkState;
    }
    
    // --- Lógica de Detecção do Botão (Corrigida) ---
    static int debouncedButtonState = HIGH; // Variável estática para manter o estado estável
    int buttonReading = digitalRead(ENCODER_SW);

    // Se a leitura atual for diferente da anterior (ruído ou ação real), reinicie o timer.
    if (buttonReading != lastButtonState) {
        lastDebounceTime = millis();
    }

    // Se o tempo desde a última mudança for maior que o debounce...
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // ...o sinal está estável. Verificamos se houve uma mudança de estado.
        if (buttonReading != debouncedButtonState) {
            debouncedButtonState = buttonReading;

            // Se o novo estado estável for "pressionado" (LOW), acionamos o evento.
            if (debouncedButtonState == LOW) {
                Serial.println("Botão PRESSIONADO");
                buttonPressed = true;
            }
        }
    }
    
    // Atualiza o último estado lido para a próxima iteração.
    lastButtonState = buttonReading;
}

int EncoderHandler::getDirection() {
    int currentDirection = direction;
    direction = 0; // Reset após leitura
    return currentDirection;
}

bool EncoderHandler::isPressed() {
    if (buttonPressed) {
        buttonPressed = false; // Reset após leitura
        return true;
    }
    return false;
}

void EncoderHandler::resetDirection() {
    direction = 0;
}