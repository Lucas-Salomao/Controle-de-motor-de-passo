# Controlador de Motor de Passo com Encoder e Display OLED

> Um sistema de controle preciso para motores de passo, desenvolvido para Arduino Nano com PlatformIO.

Este projeto oferece um controle detalhado para um motor de passo de 200 pulsos/volta (1.8° por passo), utilizando um encoder rotativo para seleção de posição e um display OLED para feedback visual. O sistema é ideal para aplicações que exigem a repetição de posições discretas e travamento do motor, como em gabaritos, sistemas de automação de pequeno porte ou equipamentos de teste.

_Última atualização: 17 de Junho de 2025_

## 📜 Descrição Geral

O firmware opera em duas fases principais:

1.  **Ciclo de Mapeamento (Automático):** Ao ser ligado, o sistema executa uma volta completa (360°) no motor de passo. A cada passo, ele aciona um relé e faz uma pausa, mapeando 200 posições distintas. A posição inicial é considerada o ponto zero, não sendo necessário um sensor de "home".
2.  **Modo de Operação (Interativo):** Após o mapeamento, o usuário pode selecionar qualquer uma das 200 posições mapeadas usando o encoder rotativo. Ao pressionar o botão do encoder, o motor se move para a posição selecionada (pelo caminho mais curto) e trava, mantendo as bobinas energizadas para garantir torque de parada. O usuário também pode desabilitar o motor para permitir o giro livre do eixo.

## ✨ Funcionalidades Principais

* **Mapeamento Automático:** Define 200 posições de trabalho em um ciclo de 360°.
* **Seleção Precisa de Posição:** O encoder rotativo permite selecionar qualquer uma das 200 posições (passos de 1.8°).
* **Movimento Otimizado:** O motor sempre gira pelo caminho mais curto até o destino.
* **Torque de Parada:** As bobinas do motor permanecem energizadas na posição de destino para resistir a movimentos externos.
* **Modo de Giro Livre:** O motor pode ser desabilitado via comando (clique longo) para movimentar o eixo manualmente.
* **Interface Clara:** O display OLED mostra o estado atual do sistema, a posição selecionada e o progresso das operações.
* **Altamente Configurável:** O código permite fácil alteração dos pinos, da lógica do driver e do tipo de display através de um arquivo de configuração.

## 🛠️ Componentes Necessários

| Componente | Quantidade | Observações |
| :--- | :--- | :--- |
| Arduino Nano | 1 | Placa principal do projeto. |
| Driver de Motor de Passo A4988 | 1 | Ou similar (DRV8825, etc.). |
| Motor de Passo NEMA 17 | 1 | Modelo de 200 passos/volta (1.8°). |
| Encoder Rotativo com Botão | 1 | Para entrada do usuário. |
| Display OLED I2C | 1 | 0.96" (SSD1306) ou 1.3" (SH1106). |
| Módulo Relé 5V | 1 | Para acionar cargas externas. |
| Fonte de Alimentação Externa | 1 | Tensão e corrente compatíveis com o motor (ex: 12V 2A). |
| Cabos Jumper | Vários | Para realizar as conexões. |

## 📦 Software e Bibliotecas

Este projeto foi desenvolvido utilizando **PlatformIO** com Visual Studio Code.

As bibliotecas necessárias são gerenciadas automaticamente pelo arquivo `platformio.ini`. Basta abrir o projeto com o PlatformIO e ele instalará tudo que for preciso.

* `waspinator/AccelStepper`: Para controle avançado e não-bloqueante do motor.
* `adafruit/Adafruit GFX Library`: Biblioteca gráfica base.
* `adafruit/Adafruit SSD1306`: Para displays de 0.96".
* `adafruit/Adafruit SH110x Library`: Para displays de 1.3".

## 🔌 Diagrama de Conexão (Wiring)

**Atenção:** A alimentação do motor (`VMOT` no A4988) **DEVE** vir de uma fonte externa. Não utilize o pino 5V do Arduino para alimentar o motor. Conecte o GND da fonte externa ao GND do Arduino.

| Componente | Pino do Componente | Pino do Arduino Nano |
| :--- | :--- | :--- |
| **Driver A4988** | `ENABLE` | `D8` |
| | `STEP` | `D2` |
| | `DIR` | `D3` |
| | `GND` | `GND` |
| | `VDD` (Lógica) | `5V` |
| | `VMOT` (Motor) | `+` da Fonte Externa (ex: 12V) |
| | `GND` (Motor) | `-` da Fonte Externa |
| **Encoder** | `CLK` | `D4` |
| | `DT` | `D5` |
| | `SW` (Botão) | `D6` |
| | `+` | `5V` |
| | `GND` | `GND` |
| **Display OLED** | `SCL` | `A5` |
| | `SDA` | `A4` |
| | `VCC` | `5V` |
| | `GND` | `GND` |
| **Módulo Relé** | `IN` (Sinal) | `D7` |
| | `VCC` | `5V` |
| | `GND` | `GND` |

## 📁 Estrutura do Projeto

O projeto está organizado da seguinte forma para facilitar a manutenção:

```
.
├── platformio.ini         // Arquivo de configuração do PlatformIO e bibliotecas
├── include
│   └── config.h           // Configurações de pinos, display e lógica do driver
└── src
    └── main.cpp           // Código principal da aplicação
```

## ⚙️ Configuração

Antes de carregar o código, você **precisa** verificar as configurações no arquivo `include/config.h` para que correspondam ao seu hardware.

### 1. Seleção do Display (Obrigatório)

Abra o `config.h` e escolha o seu tipo de display. Descomente (`#define`) a linha correspondente ao seu modelo e comente (`//`) a outra.

```cpp
// Para display de 0.96" (SSD1306)
#define DISPLAY_0_96_INCH

// Para display de 1.3" (SH1106)
// #define DISPLAY_1_3_INCH
```

### 2. Lógica do Driver do Motor (Obrigatório)

O código já está configurado para a lógica do A4988 (ativo baixo). Se você usar um driver com lógica diferente, pode alterar aqui.

```cpp
// Para A4988/DRV8825 (ativo baixo):
#define MOTOR_ENABLE_LEVEL   LOW
#define MOTOR_DISABLE_LEVEL  HIGH
```

### 3. Mapeamento de Pinos (Opcional)

Se você precisar usar outros pinos no Arduino, pode alterá-los facilmente no `config.h`.

```cpp
#define MOTOR_STEP_PIN 2
#define MOTOR_DIR_PIN 3
#define MOTOR_ENABLE_PIN 8
// ... etc
```

## 🚀 Como Usar

A operação do dispositivo é simples e direta.

1.  **Ligar o Dispositivo**
    * Ao receber energia, o display mostrará "Ciclo de Mapeamento".
    * O sistema executará uma volta completa (360°), parando em cada um dos 200 passos para acionar o relé.
    * **Aguarde** a finalização deste processo.

2.  **Modo de Operação**
    * Após o mapeamento, a tela mostrará "Modo: Selecionar Pos.".

    * 🔄 **Girar o encoder:** Altera a posição de destino (de 0 a 199), que é mostrada em tempo real no display.

    * 🖱️ **Pressionar o encoder (clique curto):**
        * O motor se moverá para a posição selecionada.
        * O display mostrará "Modo: Movimentando".
        * Ao chegar, o display mudará para "Modo: Parado / Travado" e o motor ficará energizado.

    * ⏳ **Pressionar e segurar o encoder (clique longo por >1s):**
        * Ativa o modo de giro livre. O display mostrará "!! MOTOR DESATIVADO !!".
        * As bobinas do motor serão desenergizadas, permitindo que você gire o eixo com a mão.
        * Para reativar o motor, dê um **clique curto** no botão. O sistema voltará ao modo "Parado / Travado" na última posição conhecida.

## 🔮 Melhorias Futuras

* [ ] Salvar a última posição na memória EEPROM para que não se perca ao desligar.
* [ ] Adicionar um menu de configuração para ajustar velocidade e aceleração via encoder.
* [ ] Implementar um modo de "giro contínuo" com velocidade controlada pelo encoder.
* [ ] Adicionar suporte a um sensor de fim de curso (`endstop`) para um ciclo de "homing" mais preciso.

## 📝 Licença

Este projeto é distribuído sob a Licença MIT. Veja o arquivo `LICENSE` para mais detalhes.

Copyright (c) 2025 [Lucas Salomão]