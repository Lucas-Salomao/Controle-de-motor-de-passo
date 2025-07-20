#ifndef CONFIG_H
#define CONFIG_H

// --- Pinos de Controle de Micro-passo (A4988) ---
#define MS1_PIN         14
#define MS2_PIN         12
#define MS3_PIN         13

// Configurações do Motor de Passo
#define STEP_PIN        26
#define DIR_PIN         25
#define ENABLE_PIN      27
#define BASE_STEPS_PER_REV   200
#define STEP_DELAY_US   1000  // Microsegundos entre pulsos

// Configurações do Display OLED
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C

// Defina aqui quantos itens do menu principal devem ser visíveis na tela.
// Para sua tela de 64px de altura, 3 ou 4 é um bom valor.
#define MAX_VISIBLE_MENU_ITEMS 3

// Configurações do Encoder
#define ENCODER_CLK     18
#define ENCODER_DT      19
#define ENCODER_SW      5
#define ENCODER_PULSES_PER_STEP 4

// Configurações do Relé
#define RELAY_PIN       32

// Configurações do sistema
#define ANGLE_INCREMENT 18    // 1.8 graus em décimos (18 = 1.8°)

#endif