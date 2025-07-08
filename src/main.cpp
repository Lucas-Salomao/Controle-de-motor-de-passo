#include <Arduino.h>
#include "StepperController.h"
#include "DisplayManager.h"
#include "EncoderHandler.h"
#include "config.h"
#include "logo.h"

// Protótipos das funções
void handleMainMenu();
void handleRunningCycle();
void handleAngleSetup();
void handlePositioning();
void handleMotorDisabled();
void startFullCycle();
void finishCycle();
void startPositioning();
void handleMicrostepSetup();
void applyMicrostepSetting(int setting);

// Instâncias dos controladores
StepperController stepper;
DisplayManager display;
EncoderHandler encoder;

// Variáveis de estado
enum SystemState {
  MENU_MAIN,
  RUNNING_CYCLE,
  ANGLE_SETUP,
  POSITIONING,
  MOTOR_DISABLED,
  MICROSTEP_SETUP
};

const char* menuItems[] = {
  "1. Ciclo Completo",
  "2. Posicionamento",
  "3. Micro-passo",
  "4. Desligar Motor"
  // Adicione mais itens aqui se precisar no futuro
};
const int totalMenuItems = sizeof(menuItems) / sizeof(char*);

SystemState currentState = MENU_MAIN;
int targetAngle = 0;
int currentPosition = 0; // Posição atual em steps (0-199)
bool motorEnabled = true;

// --- NOVAS VARIÁVEIS PARA MICRO-PASSO ---
// 0=Full, 1=Half, 2=1/4, 3=1/8, 4=1/16
int currentMicrostep = 0; // Inicia em Full Step por padrão
// Array com os multiplicadores de passo
const int microstepMultipliers[] = {1, 2, 4, 8, 16};
// Variável para guardar os passos por volta atuais
int activeStepsPerRev = BASE_STEPS_PER_REV;

// Variáveis para controle de timing
unsigned long lastStepTime = 0;
unsigned long relayStartTime = 0;
int cycleStep = 0; // 0: relay on, 1: relay off wait, 2: move step, 3: step wait
int cyclePosition = 0; // posição atual no ciclo completo

bool resetMenuState = false;

void setup() {
  Serial.begin(115200);

  // --- INICIALIZAÇÃO DOS PINOS DE MICRO-PASSO ---
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT);
  
  // Inicializa os componentes
  stepper.begin();
  display.begin();
  encoder.begin();
  
  // Configuração do relé
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  // Aplica a configuração de micro-passo inicial (Full Step)
  applyMicrostepSetting(currentMicrostep);
  
  Adafruit_SSD1306* logoDisplay = display.getDisplay();

  logoDisplay->clearDisplay();
  logoDisplay->drawBitmap(
      0, 0,
      epd_bitmap_logo,
      128, 64,
      SSD1306_WHITE
  );
  logoDisplay->display(); // Mostra o logo no display
  delay(2500);
  logoDisplay->clearDisplay(); // Limpa o display após mostrar o logo

  // Tela inicial
  // display.showMainMenu();

  // Tela inicial
  // MODIFICADO: Primeira chamada para a função de menu atualizada
  display.showMainMenu(menuItems, totalMenuItems, 0, 0);
  
  Serial.println("Sistema inicializado");
}

void loop() {
  // Atualiza encoder
  encoder.update();
  
  // Máquina de estados principal
  switch(currentState) {
    case MENU_MAIN:
      handleMainMenu();
      break;
      
    case RUNNING_CYCLE:
      handleRunningCycle();
      break;
      
    case ANGLE_SETUP:
      handleAngleSetup();
      break;
      
    case POSITIONING:
      handlePositioning();
      break;
      
    case MOTOR_DISABLED:
      handleMotorDisabled();
      break;

    case MICROSTEP_SETUP:
      handleMicrostepSetup();
      break;
  }
  
  delay(10); // Pequeno delay para estabilidade
}

// void handleMainMenu() {
//   static int menuIndex = 0;

//   int direction = encoder.getDirection();
  
//   // Navegação com encoder
//   if(direction != 0) {
//     menuIndex += direction; // Use a variável local aqui
//     if(menuIndex < 0) menuIndex = 3;
//     if(menuIndex > 3) menuIndex = 0;
    
//     display.showMainMenu(menuIndex);
//   }
  
//   // Seleção com botão
//   if(encoder.isPressed()) {
//     switch(menuIndex) {
//       case 0: // Ciclo completo
//         startFullCycle();
//         break;
//       case 1: // Posicionamento
//         currentState = ANGLE_SETUP;
//         targetAngle = 0;
//         display.showAngleSetup(targetAngle);
//         break;
//       case 2: // Configurar Micro-passo <-- NOVA OPÇÃO
//         currentState = MICROSTEP_SETUP;
//         display.showMicrostepSetup(currentMicrostep);
//         break;
//       case 3: // Desabilitar motor
//         stepper.disable();
//         motorEnabled = false;
//         currentState = MOTOR_DISABLED;
//         display.showMotorDisabled();
//         break;
//     }
//     delay(200); // Debounce
//   }
// }

void handleMainMenu() {
  // MODIFICADO: Variáveis de estado do menu
  static int menuIndex = 0;         // Item atualmente selecionado
  static int menuStartIndex = 0;    // Item no topo da "janela" visível

  if (resetMenuState) {
    menuIndex = 0;
    menuStartIndex = 0;
    resetMenuState = false; // Desativa o sinalizador
    // Redesenha o menu com o estado reiniciado
    display.showMainMenu(menuItems, totalMenuItems, menuIndex, menuStartIndex);
  }

  int direction = encoder.getDirection();
  
  if (direction != 0) {
    // Navega pelo índice
    menuIndex += direction;
    
    // Lógica para o índice "dar a volta" na lista
    if (menuIndex < 0) menuIndex = totalMenuItems - 1;
    if (menuIndex >= totalMenuItems) menuIndex = 0;

    // --- LÓGICA DE ROLAGEM (SCROLL) DA JANELA ---
    // Se o item selecionado estiver acima da janela visível, rola para cima
    if (menuIndex < menuStartIndex) {
      menuStartIndex = menuIndex;
    }
    // Se o item selecionado estiver abaixo da janela visível, rola para baixo
    if (menuIndex >= menuStartIndex + MAX_VISIBLE_MENU_ITEMS) {
      menuStartIndex = menuIndex - MAX_VISIBLE_MENU_ITEMS + 1;
    }
    
    // O display agora só precisa saber qual item está no topo e qual está selecionado
    display.showMainMenu(menuItems, totalMenuItems, menuIndex, menuStartIndex);
  }
  
  // A lógica de seleção não muda, continua usando o menuIndex
  if (encoder.isPressed()) {
    switch (menuIndex) {
      case 0: // Ciclo completo
        startFullCycle();
        break;
      case 1: // Posicionamento
        currentState = ANGLE_SETUP;
        targetAngle = 0;
        display.showAngleSetup(targetAngle);
        break;
      case 2: // Configurar Micro-passo
        currentState = MICROSTEP_SETUP;
        display.showMicrostepSetup(currentMicrostep);
        break;
      case 3: // Desabilitar motor
        stepper.disable();
        motorEnabled = false;
        currentState = MOTOR_DISABLED;
        display.showMotorDisabled();
        break;
    }
    delay(200);
  }
}

// Aplica a configuração de micro-passo nos pinos MS e atualiza a variável de passos
void applyMicrostepSetting(int setting) {
  // Tabela de configuração do A4988
  // Setting | MS1   | MS2   | MS3   | Resolução
  // ------------------------------------------------
  // 0 (Full)| LOW   | LOW   | LOW   | 1
  // 1 (Half)| HIGH  | LOW   | LOW   | 1/2
  // 2 (1/4) | LOW   | HIGH  | LOW   | 1/4
  // 3 (1/8) | HIGH  | HIGH  | LOW   | 1/8
  // 4 (1/16)| HIGH  | HIGH  | HIGH  | 1/16
  
  bool ms1 = (setting == 1 || setting == 3 || setting == 4);
  bool ms2 = (setting == 2 || setting == 3 || setting == 4);
  bool ms3 = (setting == 4);

  digitalWrite(MS1_PIN, ms1 ? HIGH : LOW);
  digitalWrite(MS2_PIN, ms2 ? HIGH : LOW);
  digitalWrite(MS3_PIN, ms3 ? HIGH : LOW);

  // Atualiza a variável global de passos por revolução
  activeStepsPerRev = BASE_STEPS_PER_REV * microstepMultipliers[setting];
  currentMicrostep = setting; // Atualiza o estado atual

  // Zera a posição atual, pois a referência de passos mudou
  currentPosition = 0;
  
  Serial.printf("Micro-passo configurado para: %dx\n", microstepMultipliers[setting]);
  Serial.printf("Passos por volta agora: %d\n", activeStepsPerRev);
}

// Lida com a tela de configuração de micro-passo
void handleMicrostepSetup() {
  static int selectedMicrostep = currentMicrostep;

  int direction = encoder.getDirection();
  if (direction != 0) {
    selectedMicrostep += direction;
    if (selectedMicrostep < 0) selectedMicrostep = 4;
    if (selectedMicrostep > 4) selectedMicrostep = 0;
    display.showMicrostepSetup(selectedMicrostep);
  }

  if (encoder.isPressed()) {
    applyMicrostepSetting(selectedMicrostep);
    currentState = MENU_MAIN;
    // display.showMainMenu();
    resetMenuState = true;
    delay(200);
  }
}

void startFullCycle() {
  currentState = RUNNING_CYCLE;
  cycleStep = 0;
  cyclePosition = 0;
  currentPosition = 0;
  stepper.enable();
  motorEnabled = true;
  
  // Inicia o primeiro passo do ciclo
  digitalWrite(RELAY_PIN, LOW);
  relayStartTime = millis();
  
  display.showCycleProgress(cyclePosition, activeStepsPerRev);
  Serial.println("Iniciando ciclo completo");
}

void handleRunningCycle() {
  unsigned long currentTime = millis();
  
  switch(cycleStep) {
    case 0: // Relé ligado por 0.5s
      if(currentTime - relayStartTime >= 500) {
        digitalWrite(RELAY_PIN, HIGH);
        lastStepTime = currentTime;
        cycleStep = 1;
      }
      break;
      
    case 1: // Aguarda 0.5s com relé desligado
      if(currentTime - lastStepTime >= 500) {
        // Move um passo
        stepper.moveOneStep();
        currentPosition = (currentPosition + 1) % activeStepsPerRev;
        cyclePosition++;
        lastStepTime = currentTime;
        cycleStep = 2;
        
        display.showCycleProgress(cyclePosition, activeStepsPerRev);
      }
      break;
      
    case 2: // Aguarda após movimento
      if(currentTime - lastStepTime >= 10) { // Pequena pausa após movimento
        if(cyclePosition >= activeStepsPerRev) {
          // Ciclo completo finalizado
          finishCycle();
        } else {
          // Próximo passo do ciclo
          digitalWrite(RELAY_PIN, LOW);
          relayStartTime = currentTime;
          cycleStep = 0;
        }
      }
      break;
  }
  
  // Permite cancelar com botão
  if(encoder.isPressed()) {
    digitalWrite(RELAY_PIN, HIGH);
    currentState = MENU_MAIN;
    // display.showMainMenu();
    resetMenuState = true;
    Serial.println("Ciclo cancelado");
    delay(200);
  }
}

void finishCycle() {
  currentState = MENU_MAIN;
  display.showCycleComplete();
  delay(2000);
  resetMenuState = true;
  Serial.println("Ciclo completo finalizado");
}

void handleAngleSetup() {
  int direction = encoder.getDirection();
  // Ajuste do ângulo com encoder
  if(direction != 0) {
    targetAngle += direction * 18; // Incremento de 1.8 graus (18 décimos)
    
    // Limita entre 0 e 360 graus
    if(targetAngle < 0) targetAngle = 3582; // 358.2 graus
    if(targetAngle >= 3600) targetAngle = 0;
    
    display.showAngleSetup(targetAngle);
  }
  
  if(encoder.isPressed()) {
    // Confirma ângulo e inicia posicionamento
    currentState = POSITIONING;
    startPositioning();
    delay(200);
  }
}

void startPositioning() {
  stepper.enable();
  motorEnabled = true;

  float anglePerStep = 360.0 / activeStepsPerRev;
  
  // Calcula posição alvo em steps
  int targetStep = round((targetAngle / 10.0) / anglePerStep);
  // Calcula menor caminho
  int stepsToMove = targetStep - currentPosition;

  int halfWay = activeStepsPerRev / 2;
  
  if(stepsToMove > halfWay) {
    stepsToMove -= activeStepsPerRev;
  } else if(stepsToMove < -halfWay) {
    stepsToMove += activeStepsPerRev;
  }
  
  display.showPositioning(targetAngle, stepsToMove);
  
  // Executa movimento
  if(stepsToMove != 0) {
    stepper.moveSteps(stepsToMove);
    currentPosition = targetStep;
  }
  
  // Mantém motor energizado para travar posição
  Serial.printf("Posicionado em %.1f graus (%d steps)\n", targetAngle/10.0, currentPosition);
  
  // Retorna ao menu após 2 segundos
  delay(2000);
  currentState = MENU_MAIN;
  resetMenuState = true;
}

void handlePositioning() {
  // Estado transitório - o posicionamento é feito imediatamente
  // Este método existe para futuras expansões
}

void handleMotorDisabled() {
  if(encoder.isPressed()) {
    // Reabilita motor e volta ao menu
    stepper.enable();
    motorEnabled = true;
    currentState = MENU_MAIN;
    resetMenuState = true;
    Serial.println("Motor reabilitado");
    delay(200);
  }
}


// ====== TESTE 1: MOTOR DE PASSO ======
// #include <Arduino.h>

// void setup() {
//     Serial.begin(115200);
    
//     pinMode(26, OUTPUT); // STEP
//     pinMode(25, OUTPUT); // DIR  
//     pinMode(27, OUTPUT); // ENABLE
    
//     digitalWrite(27, LOW); // Habilita motor
//     digitalWrite(25, LOW); // Direção horária
    
//     Serial.println("Teste do motor de passo iniciado");
// }

// void loop() {
//     // Move 10 passos
//     for(int i = 0; i < 10000; i++) {
//         digitalWrite(26, HIGH);
//         delayMicroseconds(500);
//         digitalWrite(26, LOW);
//         delayMicroseconds(500);
//         Serial.printf("Passo %d\n", i+1);
//     }
    
//     delay(2000);
    
//     // Inverte direção
//     digitalWrite(25, !digitalRead(25));
//     Serial.println("Invertendo direção");
// }

// ====== TESTE 3: ENCODER ROTATIVO ======
// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// void setup() {
//     Serial.begin(115200);
    
//     pinMode(18, INPUT_PULLUP); // CLK
//     pinMode(19, INPUT_PULLUP); // DT
//     pinMode(5, INPUT_PULLUP);  // SW
    
//     Serial.println("Teste do encoder iniciado");
//     Serial.println("Gire o encoder e pressione o botão");
// }

// void loop() {
//     static int lastClk = HIGH;
//     static int lastButton = HIGH;
    
//     int clk = digitalRead(18);
//     int dt = digitalRead(19);
//     int button = digitalRead(5);
    
//     // Detecta rotação
//     if(clk != lastClk) {
//         if(dt != clk) {
//             Serial.println("Encoder: HORÁRIO");
//         } else {
//             Serial.println("Encoder: ANTI-HORÁRIO");
//         }
//         lastClk = clk;
//     }
    
//     // Detecta botão
//     if(button == LOW && lastButton == HIGH) {
//         Serial.println("Botão PRESSIONADO");
//         delay(200); // Debounce
//     }
//     lastButton = button;
    
//     delay(10);
// }