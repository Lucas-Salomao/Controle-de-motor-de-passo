#include <Arduino.h>
#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_GFX.h>
#include "config.h"

// --- Inclusão Condicional da Biblioteca do Display ---
#if defined(DISPLAY_0_96_INCH)
  #include <Adafruit_SSD1306.h>
#elif defined(DISPLAY_1_3_INCH)
  #include <Adafruit_SH110x.h>
#else
  #error "Nenhum tipo de display foi selecionado no arquivo config.h"
#endif

// --- Máquina de Estados para controlar o fluxo do programa ---
enum State {
  STATE_MAPPING,        // Executando o ciclo inicial de mapeamento
  STATE_SELECT_POS,     // Usuário está escolhendo uma posição
  STATE_MOVING,         // Motor se movendo para a posição alvo
  STATE_HOLDING,        // Motor parado na posição, bobinas energizadas
  STATE_DISABLED        // Motor desativado, giro livre
};

// --- Instância Condicional do Objeto do Display ---
#if defined(DISPLAY_0_96_INCH)
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#elif defined(DISPLAY_1_3_INCH)
  Adafruit_SH110x display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 500000L); // 500kHz I2C
#endif

State currentState = STATE_MAPPING;

// --- Instância dos Objetos ---
AccelStepper stepper(AccelStepper::DRIVER, MOTOR_STEP_PIN, MOTOR_DIR_PIN);

// --- Variáveis Globais ---
volatile long encoderPos = 0;
long lastEncoderPos = 0;
long targetPosition = 0;

// Variáveis para controle do botão
int lastButtonState = HIGH;
unsigned long pressStartTime = 0;
bool buttonHeld = false;


// --- Protótipos das Funções ---
void updateDisplay();
void handleEncoder();
void handleButton();
void runMappingCycle();
void readEncoderISR();
void enableMotor();
void disableMotor();

void setup() {
  Serial.begin(115200);

  // --- Configuração dos Pinos ---
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  pinMode(ENCODER_CLK_PIN, INPUT);
  pinMode(ENCODER_DT_PIN, INPUT);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // --- Inicialização Condicional do Display ---
  bool display_ok = false;
  #if defined(DISPLAY_0_96_INCH)
    display_ok = display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
  #elif defined(DISPLAY_1_3_INCH)
    display_ok = display.begin(I2C_ADDRESS, true); // true = resetar
  #endif

  if (!display_ok) {
    Serial.println(F("Falha ao iniciar display OLED"));
    while (true);
  }
  
  // --- Configuração do Motor ---
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(500.0);
  enableMotor(); // Garante que o motor comece habilitado

  // --- Configuração da Interrupção do Encoder ---
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), readEncoderISR, CHANGE);

  // --- O programa começa executando o ciclo de mapeamento ---
  runMappingCycle();
  
  // Após o mapeamento, muda para o modo de seleção de posição
  currentState = STATE_SELECT_POS;
  stepper.setCurrentPosition(0); // A posição final do mapeamento é o nosso novo '0'
  encoderPos = 0; // Reseta o encoder
  lastEncoderPos = 0;
  targetPosition = 0;
  updateDisplay();
}

void loop() {
  // Funções que rodam independentemente do estado
  handleEncoder();
  handleButton();

  // O motor só precisa ser atualizado se não estiver desabilitado
  if (currentState != STATE_DISABLED) {
    stepper.run();
  }

  // Lógica principal baseada no estado atual
  switch (currentState) {
    case STATE_MOVING:
      if (stepper.distanceToGo() == 0) {
        currentState = STATE_HOLDING;
        updateDisplay();
      }
      break;
    
    // Outros estados são alterados por eventos (botão, encoder)
    case STATE_MAPPING:
    case STATE_SELECT_POS:
    case STATE_HOLDING:
    case STATE_DISABLED:
      // Nenhuma ação contínua necessária no loop
      break;
  }
}

/**
 * @brief Executa o ciclo inicial de mapeamento de 360°.
 * @note Esta função é BLOQUEANTE.
 */
void runMappingCycle() {
  updateDisplay(); // Mostra a mensagem "Mapeando..."

  stepper.setCurrentPosition(0); // A posição inicial é a nossa referência 0

  for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
    // 1. Aciona o relé por 0.5s
    digitalWrite(RELAY_PIN, HIGH);
    delay(RELAY_ON_TIME_MS);
    digitalWrite(RELAY_PIN, LOW);

    // 2. Aguarda mais 0.5s
    delay(PAUSE_AFTER_RELAY_MS);

    // 3. Movimenta o motor um passo
    stepper.move(1);
    stepper.runToPosition(); // Espera o movimento terminar

    // Atualiza o display com o progresso
    display.clearDisplay();
    display.setCursor(0, 16);
    display.setTextSize(1);
    display.print("Mapeando Passo: ");
    display.print(i + 1);
    display.print("/");
    display.print(STEPS_PER_REVOLUTION);
    display.display();
  }

  // 4. Última ativação do relé na posição final
  digitalWrite(RELAY_PIN, HIGH);
  delay(RELAY_ON_TIME_MS);
  digitalWrite(RELAY_PIN, LOW);
  delay(PAUSE_AFTER_RELAY_MS);
}

/**
 * @brief Gerencia a entrada do encoder rotativo.
 */
void handleEncoder() {
  if (encoderPos != lastEncoderPos) {
    if (currentState == STATE_SELECT_POS || currentState == STATE_HOLDING) {
      // Limita a seleção entre 0 e 199 posições
      targetPosition = encoderPos % STEPS_PER_REVOLUTION;
      if (targetPosition < 0) {
        targetPosition += STEPS_PER_REVOLUTION;
      }
      
      currentState = STATE_SELECT_POS; // Se estava parado, agora está selecionando
      updateDisplay();
    }
    lastEncoderPos = encoderPos;
  }
}

/**
 * @brief Gerencia os cliques do botão (curto para ação, longo para habilitar/desabilitar).
 */
void handleButton() {
  int reading = digitalRead(ENCODER_SW_PIN);

  // Lógica para clique longo
  if (reading == LOW && lastButtonState == HIGH) { // Botão acabou de ser pressionado
    pressStartTime = millis();
    buttonHeld = true;
  }

  if (reading == HIGH && lastButtonState == LOW) { // Botão acabou de ser solto
    if (millis() - pressStartTime < LONG_PRESS_TIME_MS) {
      // --- CLIQUE CURTO ---
      if (currentState == STATE_SELECT_POS) {
        stepper.moveTo(targetPosition);
        currentState = STATE_MOVING;
      } else if (currentState == STATE_DISABLED) {
        enableMotor();
        currentState = STATE_HOLDING;
      }
      updateDisplay();
    }
    buttonHeld = false;
  }

  // Verifica se o botão está sendo segurado por tempo suficiente
  if (buttonHeld && (millis() - pressStartTime > LONG_PRESS_TIME_MS)) {
    // --- CLIQUE LONGO ---
    if (currentState != STATE_DISABLED) {
      disableMotor();
      currentState = STATE_DISABLED;
    }
    // Para reativar, é preciso soltar e dar um clique curto
    buttonHeld = false; // Evita múltiplas execuções
    updateDisplay();
  }
  
  lastButtonState = reading;
}


/**
 * @brief Atualiza o display OLED com base no estado atual.
 */
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  switch (currentState) {
    case STATE_MAPPING:
      display.println("Ciclo de Mapeamento");
      display.println("---------------------");
      display.println("Aguarde...");
      break;
    
    case STATE_SELECT_POS:
      display.println("Modo: Selecionar Pos.");
      display.println("---------------------");
      display.setTextSize(2);
      display.print("Pos: ");
      display.print(targetPosition);
      break;

    case STATE_MOVING:
      display.println("Modo: Movimentando");
      display.println("---------------------");
      display.setTextSize(2);
      display.print("-> ");
      display.print(targetPosition);
      break;

    case STATE_HOLDING:
      display.println("Modo: Parado / Travado");
      display.println("---------------------");
      display.setTextSize(2);
      display.print("Pos: ");
      display.print(stepper.currentPosition());
      break;
      
    case STATE_DISABLED:
      display.println("!! MOTOR DESATIVADO !!");
      display.println("---------------------");
      display.println();
      display.println("Giro livre.");
      display.println("Clique para reativar.");
      break;
  }
  display.display();
}

void enableMotor() {
  digitalWrite(MOTOR_ENABLE_PIN, MOTOR_ENABLE_LEVEL); // Usa a constante do config.h
  Serial.println("Motor Habilitado.");
}

void disableMotor() {
  digitalWrite(MOTOR_ENABLE_PIN, MOTOR_DISABLE_LEVEL); // Usa a constante do config.h
  Serial.println("Motor Desabilitado.");
}


/**
 * @brief ISR para ler o encoder. Deve ser o mais rápido possível.
 */
void readEncoderISR() {
  if (digitalRead(ENCODER_DT_PIN) != digitalRead(ENCODER_CLK_PIN)) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}