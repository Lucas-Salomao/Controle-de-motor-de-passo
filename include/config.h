#pragma once // Evita que o arquivo seja incluído múltiplas vezes

// --- Pinos do Motor de Passo (Driver A4988 ou similar) ---
// Conecte os pinos do driver aos pinos digitais do Arduino
#define MOTOR_STEP_PIN 2
#define MOTOR_DIR_PIN 3
#define MOTOR_ENABLE_PIN 8

// --- Configuração da Lógica do Pino ENABLE ---
// Defina o nível lógico para Habilitar e Desabilitar o motor.
// Para A4988/DRV8825 (ativo baixo): ENABLE = LOW, DISABLE = HIGH
// Para outros drivers (ativo alto):  ENABLE = HIGH, DISABLE = LOW
#define MOTOR_ENABLE_LEVEL   LOW
#define MOTOR_DISABLE_LEVEL  HIGH

// --- Pinos do Encoder Rotativo ---
#define ENCODER_CLK_PIN 4 // Pino CLK do encoder
#define ENCODER_DT_PIN 5  // Pino DT do encoder
#define ENCODER_SW_PIN 6  // Pino do botão (switch)

// --- Pino do Relé ---
#define RELAY_PIN 7

// =========================================================================
// --- CONFIGURAÇÃO DO TIPO DE DISPLAY ---
// Descomente APENAS UMA das linhas abaixo, dependendo do seu display.
//
// DISPLAY_0_96_INCH: Para displays de 0.96" (128x64, controlador SSD1306)
// DISPLAY_1_3_INCH: Para displays de 1.3" (128x64, controlador SH1106)
// =========================================================================
#define DISPLAY_0_96_INCH
// #define DISPLAY_1_3_INCH

// --- Configurações do Display OLED I2C ---
#define SCREEN_WIDTH 128 // Largura do OLED em pixels
#define SCREEN_HEIGHT 64 // Altura do OLED em pixels
#define OLED_RESET -1    // Pino de reset (-1 se estiver compartilhando o pino de reset do Arduino)
#define I2C_ADDRESS 0x3C // Endereço I2C mais comum para ambos os displays

// --- Configurações do Motor ---
#define STEPS_PER_REVOLUTION 200 // 200 passos por volta (motor de 1.8 graus)
#define DEGREES_PER_STEP 1.8     // 360 / 200 = 1.8

// --- Configurações de Funcionamento ---
#define RELAY_ON_TIME_MS 500  // Tempo que o relé fica acionado em milissegundos
#define PAUSE_AFTER_RELAY_MS 500 // Pausa após o acionamento do relé em milissegundos
#define LONG_PRESS_TIME_MS 1000 // Tempo para considerar um "clique longo"