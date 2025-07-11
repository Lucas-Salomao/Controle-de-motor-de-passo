#include "DisplayManager.h"

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

void DisplayManager::begin() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("Falha na inicialização do display SSD1306");
        return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    // display.println("Inicializando...");
    display.display();
    
    // delay(1000);
    
    Serial.println("Display inicializado");
}

void DisplayManager::clear() {
    display.clearDisplay();
}

// void DisplayManager::showMainMenu(int selectedIndex) {
//     clear();
    
//     display.setTextSize(1);
//     display.setCursor(0, 0);
//     display.println("===MENU PRINCIPAL===");
//     display.println();
    
//     // Opção 1: Ciclo Completo
//     if(selectedIndex == 0) display.print("> ");
//     else display.print("  ");
//     display.println("1. Ciclo Completo");
    
//     // Opção 2: Posicionamento
//     if(selectedIndex == 1) display.print("> ");
//     else display.print("  ");
//     display.println("2. Posicionamento");

//     if(selectedIndex == 2) display.print("> ");
//     else display.print("  ");
//     display.println("3. Micro-passo");
    
//     // Opção 3: Desabilitar Motor
//     if(selectedIndex == 3) display.print("> ");
//     else display.print("  ");
//     display.println("4. Desligar Motor");
    
//     display.println();
//     display.println("Gire: Navegar");
//     display.println("Clique: Selecionar");
    
//     display.display();
// }

// --- FUNÇÃO showMainMenu COMPLETAMENTE REFEITA ---
void DisplayManager::showMainMenu(const char* items[], int totalItems, int selectedIndex, int startIndex) {
    clear();
    
    const int yOffset = 18;         // Posição Y inicial para o primeiro item
    const int lineHeight = 10;      // Altura de cada linha do menu

    // Desenha o título
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("===MENU PRINCIPAL===");
    
    // Desenha os itens do menu que estão na "janela"
    for (int i = 0; i < MAX_VISIBLE_MENU_ITEMS; i++) {
        int itemIndex = startIndex + i;
        if (itemIndex >= totalItems) {
            break; // Para de desenhar se chegamos ao fim da lista
        }

        display.setCursor(0, yOffset + (i * lineHeight));

        // Se o item atual do loop for o selecionado, desenha o cursor ">"
        if (itemIndex == selectedIndex) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.println(items[itemIndex]);
    }

    // --- Adiciona indicadores de rolagem (setas) ---
    // Se não estamos no topo da lista, mostra a seta para cima
    if (startIndex > 0) {
        display.setCursor(120, 10); // Canto superior direito
        display.print((char)30); // Caractere ASCII para a seta para cima (▲)
    }
    // Se há mais itens para baixo, mostra a seta para baixo
    if (startIndex + MAX_VISIBLE_MENU_ITEMS < totalItems) {
        display.setCursor(120, 54); // Canto inferior direito
        display.print((char)31); // Caractere ASCII para a seta para baixo (▼)
    }

    // --- ADIÇÃO DO TEXTO DE AJUDA ---
    // Posiciona o texto de ajuda na parte inferior da tela.
    // As coordenadas Y (48 e 56) funcionam bem para uma tela de 64 pixels de altura.
    display.setCursor(0, 48);
    display.println("Gire: Navegar");
    display.setCursor(0, 56);
    display.println("Clique: Selecionar");
    // --- FIM DA ADIÇÃO ---
    
    display.display();
}

void DisplayManager::showCycleProgress(int currentStep, int totalSteps) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== CICLO ATIVO ===");
    display.println();
    
    display.printf("Passo: %d/%d\n", currentStep, totalSteps);
    display.printf("Progresso: %.1f%%\n", (currentStep * 100.0) / totalSteps);
    
    // Barra de progresso
    int barWidth = 100;
    int barHeight = 8;
    int progress = (currentStep * barWidth) / totalSteps;
    
    display.drawRect(14, 35, barWidth + 2, barHeight + 2, SSD1306_WHITE);
    display.fillRect(15, 36, progress, barHeight, SSD1306_WHITE);
    
    display.setCursor(0, 50);
    display.println("Clique: Cancelar");
    
    display.display();
}

void DisplayManager::showMicrostepSetup(int selectedIndex) {
    clear();
    const char* options[] = {"Full Step", "1/2 Step", "1/4 Step", "1/8 Step", "1/16 Step"};

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== MICRO-PASSO ===");
    
    // Mostra 5 opções na tela
    for (int i = 0; i < 5; i++) {
      display.setCursor(0, 10 + (i * 10));
      if (i == selectedIndex) {
        display.print("> ");
      } else {
        display.print("  ");
      }
      display.println(options[i]);
    }

    display.display();
}

void DisplayManager::showCycleComplete() {
    clear();
    
    display.setTextSize(2);
    display.setCursor(10, 15);
    display.println("CICLO");
    display.setCursor(0, 35);
    display.println("COMPLETO!");
    
    display.display();
}

// void DisplayManager::showAngleSetup(int angle) {
//     clear();
    
//     display.setTextSize(1);
//     display.setCursor(0, 0);
//     display.println("=== CONFIG ANGULO ===");
//     display.println();
    
//     display.setTextSize(2);
//     display.setCursor(15, 20);
//     display.printf("%.1f", angle / 10.0);
//     display.println(" deg");
    
//     display.setTextSize(1);
//     display.setCursor(0, 45);
//     display.println("Gire: Ajustar");
//     display.println("Clique: Confirmar");
    
//     display.display();
// }

void DisplayManager::showPositioning(int targetStep, int stepsToMove) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== POSICIONANDO ===");
    display.println();
    
    display.printf("Alvo: Passo %d\n", targetStep);
    display.printf("Steps: %d\n", abs(stepsToMove));
    display.printf("Direcao: %s\n", stepsToMove >= 0 ? "Horario" : "Anti-hor.");
    
    display.println();
    display.println("Posicionando...");
    
    display.display();
}

void DisplayManager::showMotorDisabled() {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== MOTOR LIVRE ===");
    // display.println();
    
    display.setTextSize(2);
    display.setCursor(30, 15);
    display.println("MOTOR");
    display.setCursor(10, 35);
    display.println("DESLIGADO");
    
    display.setTextSize(1);
    display.setCursor(0, 53);
    display.println("Clique: Habilitar");
    
    display.display();
}

void DisplayManager::showError(const char* message) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== ERRO ===");
    display.println();
    
    display.println(message);
    
    display.display();
}

void DisplayManager::showRelayTimeSetup(int timeMs) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== TEMPO DO RELE ===");
    display.println();
    
    // Mostra o tempo formatado em segundos
    display.setTextSize(2);
    display.setCursor(15, 20);
    display.printf("%.2f s", timeMs / 1000.0);
    
    // Instruções
    display.setTextSize(1);
    display.setCursor(0, 45);
    display.println("Gire: Ajustar");
    display.println("Clique: Confirmar");
    
    display.display();
}

void DisplayManager::showPositioningSetup(int steps) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== POSICAO FINAL ===");
    
    display.setTextSize(2);
    display.setCursor(15, 20);
    display.printf("Passo: %d", steps);
    
    display.setTextSize(1);
    display.setCursor(0, 48);
    display.println("Gire: Ajustar Pos.");
    display.setCursor(0, 56);
    display.println("Clique: Confirmar");
    display.display();
}

Adafruit_SSD1306* DisplayManager::getDisplay() {
    return &display;
}