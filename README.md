```markdown
# 🕹️ Tetris ESP32 — 60 FPS & Joystick Analógico

Implementação modular em **C++ orientado a objetos** do clássico jogo **Tetris** para o microcontrolador **ESP32 DevKit V1**, utilizando um display colorido **TFT IPS 1.54" ST7789 (240x240)** via SPI de hardware e controle por **Joystick Analógico (KY-023)** com temporização determinística travada em **60 FPS**.

---

## ✨ Funcionalidades e Arquitetura

* **Loop Determinístico a 60 FPS:** Controle rigoroso de taxa de quadros via `micros()` ($16.666\ \mu\text{s}$ por quadro) com temporização baseada em contadores de frames inteiros (*Frame-Based Timing*).
* **Renderização Parcial Sem Flicker:** Atualiza apenas os blocos alterados no tabuleiro usando a biblioteca `TFT_eSPI`, mantendo resposta visual instantânea.
* **Lógica Baseada em Matrizes:**
  * Matriz fixa do tabuleiro (`20x10`).
  * Moldes das 7 peças clássicas (`I, J, L, O, S, T, Z`) alinhados no topo (`4x4`) com rotação por transposição de matriz.
  * Detecção de colisão lateral, fundo, peças fixas, *Block Out* e *Lock Out* (Game Over).
* **Progressão Dinâmica de Dificuldade:** A velocidade de queda aumenta automaticamente a cada 500 pontos acumulados.
* **Código Modular:** Separado entre interface (`include/`) e implementação (`src/`), documentado no padrão Doxygen.

---

## 🛠️ Lista de Componentes (BOM)

| Componente | Especificação | Qtd. | Referência Visual |
| :--- | :--- | :---: | :--- |
| **Microcontrolador** | ESP32-WROOM-32 DevKit V1 (30 pinos) | 1 | ![ESP32 DevKit](https://upload.wikimedia.org/wikipedia/commons/thumb/a/a2/ESP32_DevKitC_V4.jpg/320px-ESP32_DevKitC_V4.jpg) |
| **Display Gráfico** | LCD TFT IPS 1.54" ST7789 ($240 \times 240$, SPI, 3.3V) | 1 | ![Display TFT SPI](https://upload.wikimedia.org/wikipedia/commons/thumb/4/4e/1.8_inch_TFT_LCD_display_module.jpg/320px-1.8_inch_TFT_LCD_display_module.jpg) |
| **Controle** | Módulo Joystick Analógico 2 Eixos + Botão (KY-023) | 1 | ![Joystick KY-023](https://upload.wikimedia.org/wikipedia/commons/thumb/b/b5/Thumb_joystick_module.jpg/320px-Thumb_joystick_module.jpg) |
| **Prototipagem** | Protoboard (830 pontos) + Jumpers Macho-Fêmea / Macho-Macho | - | ![Protoboard](https://upload.wikimedia.org/wikipedia/commons/thumb/7/73/400_points_breadboard.jpg/320px-400_points_breadboard.jpg) |
| **Alimentação** | Cabo Micro-USB / USB-C de dados conectado ao PC ou fonte 5V | 1 | — |

---

## 🔌 Diagrama de Ligação (Pinagem)

> ⚠️ **Atenção:** Alimente tanto o display ST7789 quanto o módulo Joystick utilizando exclusivamente o pino **3V3** do ESP32. O ADC do ESP32 opera na faixa de `0V a 3.3V` e os pinos analógicos do joystick devem ser ligados nos canais **ADC1** (`GPIO 34` e `GPIO 35`).

### 1. Display TFT IPS 1.54" ST7789 (Barramento VSPI)

| Pino no Display | Serigrafia no ESP32 | GPIO no Código | Função |
| :--- | :--- | :---: | :--- |
| **GND** | `GND` | `GND` | Terra comum |
| **VCC** | `3V3` | `3.3V` | Alimentação lógica |
| **SCL / SCK** | `D18` | `18` | Clock SPI (*VSPI SCK*) |
| **SDA / MOSI** | `D23` | `23` | Dados SPI (*VSPI MOSI*) |
| **RES / RST** | `D4` | `4` | Reset do controlador |
| **DC** | `D2` | `2` | Seleção Comando / Dado |
| **CS** *(opcional)* | `D5` | `5` | Chip Select *(ignore se o módulo tiver 7 pinos)* |
| **BLK / LED** | `3V3` | `3.3V` | Luz de fundo (*Backlight* 100%) |

### 2. Módulo Joystick Analógico (KY-023)

| Pino no Joystick | Serigrafia no ESP32 | GPIO no Código | Função |
| :--- | :--- | :---: | :--- |
| **GND** | `GND` | `GND` | Terra comum |
| **+5V / VCC** | `3V3` | `3.3V` | Referência de tensão de 3.3V |
| **VRx** | `D34` | `34` | Eixo Horizontal (Esquerda / Direita) |
| **VRy** | `D35` | `35` | Eixo Vertical (Cima = Girar / Baixo = Cair rápido) |
| **SW** | `D32` | `32` | Botão central do manche (Girar / Reiniciar) |

---

## 📂 Estrutura do Projeto

```text
tetris_esp32/
├── include/
│   ├── config.h          # Pinagem, dimensões da tela, FPS e moldes das peças
│   └── tetris.h          # Declaração da classe TetrisGame (documentação Doxygen)
├── src/
│   ├── main.cpp          # Setup de hardware, leitura do Joystick e loop a 60 FPS
│   └── tetris.cpp        # Motor lógico, colisões, rotação e renderização parcial
├── test/
│   ├── mocks/            # Mocks de Arduino.h e TFT_eSPI.h para testes nativos
│   ├── test_logic/       # Testes unitários da lógica do jogo (Unity)
│   └── test_integration/ # Testes de integração do game loop (Unity)
├── platformio.ini        # Configuração da placa, dependências e flags da TFT_eSPI
└── README.md

```

## 🚀 Como Compilar e Gravar com PlatformIO (`pio`)

### Pré-requisitos

1. **PlatformIO Core (CLI)** instalado ou extensão **PlatformIO IDE** no VS Code.
2. *(Linux)* Permissões de acesso à porta serial configuradas para o seu usuário:
```bash
sudo usermod -aG dialout $USER

```


*(Reinicie a sessão após executar o comando caso seja a primeira vez).*

### Passo 1: Clonar o Repositório

```bash
git clone [https://github.com/seu-usuario/tetris-esp32.git](https://github.com/seu-usuario/tetris-esp32.git)
cd tetris-esp32

```

### Passo 2: Conferir o `platformio.ini`

O projeto já inclui todas as flags necessárias da `TFT_eSPI` (incluindo `-D LOAD_GLCD=1` para renderização da fonte do placar):

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200

lib_deps =
    bodmer/TFT_eSPI @ ^2.5.43

build_flags =
    -D DISABLE_ALL_LIBRARY_WARNINGS=1
    -D USER_SETUP_LOADED=1
    -D ST7789_DRIVER=1
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=240
    -D TFT_MOSI=23
    -D TFT_SCLK=18
    -D TFT_CS=5
    -D TFT_DC=2
    -D TFT_RST=4
    -D LOAD_GLCD=1
    -D LOAD_FONT2=1
    -D SPI_FREQUENCY=40000000

```

### Passo 3: Compilar o Firmware (Build)

Na raiz do projeto, execute o comando abaixo para baixar automaticamente a biblioteca `TFT_eSPI` e compilar o código:

```bash
pio run

```

### Passo 4: Gravar na Placa (Upload)

Conecte o ESP32 DevKit V1 na porta USB e execute:

```bash
pio run -t upload

```

> **Dica de Gravação:** Se o terminal ficar aguardando em `Connecting........_____.....`, mantenha pressionado o botão físico **BOOT** na placa do ESP32 até o início da porcentagem de escrita (`Writing at 0x00001000...`) e solte em seguida.

### Passo 5: Monitor Serial (Opcional)

Para acompanhar logs de inicialização:

```bash
pio device monitor -b 115200

```

---

## 🧪 Testes Automatizados

O projeto inclui **testes unitários** (`test/test_logic/`) e **de integração** (`test/test_integration/`) escritos com o framework **Unity** (embutido no PlatformIO). Eles rodam **no PC** (ambiente `native`), sem necessidade do ESP32, usando mocks do core Arduino e da `TFT_eSPI` (`test/mocks/`) que simulam joystick, botão, relógio (`micros()`) e sorteio de peças.

```bash
pio test -e native

```

* **Unitários:** colisão (paredes, chão e blocos fixos), rotação (incluindo rotação bloqueada), limpeza de linhas, pontuação/nível, trava de peças e condições de *Game Over*.
* **Integração:** dirigem o `Tetris::update()` quadro a quadro — gravidade a 60 FPS, movimento pelo joystick, *soft drop*, rotação *edge-triggered*, queda completa com spawn da próxima peça, empilhamento até *Game Over* e reinício pelo botão.

---

## 🎮 Controles no Jogo

* **Mover para Esquerda / Direita:** Empurrar o analógico no eixo **X**.
* **Rotacionar Peça (90° horário):** Empurrar o analógico para **Cima** ou pressionar o botão central (**SW**).
* **Queda Rápida (*Soft Drop*):** Segurar o analógico para **Baixo**.
* **Reiniciar após Game Over:** Pressionar o botão central (**SW**) ou empurrar o analógico para **Cima**.

---

## 🔧 Calibração Rápida (Caso os Eixos Fiquem Invertidos)

Dependendo da orientação física em que o módulo joystick for encaixado na protoboard, caso **Esquerda/Direita** ou **Cima/Baixo** fiquem invertidos, basta inverter as comparações `< DEADZONE_LOW` e `> DEADZONE_HIGH` dentro de `TetrisGame::update()` no arquivo `src/tetris.cpp`.

```

```