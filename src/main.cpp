#include <Arduino.h>

// Include necessary libraries for TFT display and SPI communication
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// Frames per second for the game loop
const int FPS = 60;
const unsigned long DELTA_FRAME_TIME = 1000000 / FPS; // 16.666 microssegundos
unsigned long lastFrameTime = 0;

int moveFrameCounter = 0; // Counter for move delay frames
int fallFrameCounter = 0; // Counter for fall delay frames

const int MOVE_DELAY_FRAMES = 7; // Number of frames to wait before allowing another move
const int DROP_DELAY_FRAMES = 3; // Number of frames to wait before allowing another fall
int gravityFrames = 30; // Number of frames for gravity to act on the piece (0.5 seconds at 60 FPS)

// Serial communication baud rate
const int SERIAL_BAUD_RATE = 115200;

// Fall speed in milliseconds
const int NORMAL_FALL_SPEED = 50;
const int FAST_FALL_SPEED = 500;

// Joystick pins and variables
const int JOYSTICK_X_PIN = 34;
const int JOYSTICK_Y_PIN = 35;
const int JOYSTICK_BUTTON_PIN = 32;

// Deadzone for joystick
const int JOYSTICK_DEADZONE_LOW = 1000;
const int JOYSTICK_DEADZONE_HIGH = 3000;

// Board width and height
const int BOARD_WIDTH = 240;
const int BOARD_HEIGHT = 240;
const int BOARD_COLS = 10;
const int BOARD_ROWS = 20;
const int BLOCK_SIZE = 11;
const int BOARD_X = 15;
const int BOARD_Y = 10;

// Game board array
uint8_t board[BOARD_ROWS][BOARD_COLS] = {0};

const uint16_t COLORS[8] = {
    TFT_BLACK,
    TFT_CYAN,
    TFT_BLUE,
    TFT_ORANGE,
    TFT_YELLOW,
    TFT_GREEN,
    TFT_MAGENTA,
    TFT_RED,
};

const uint8_t SHAPES[7][4][4] = {
    {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // J
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // L
    {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // O
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // S
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // T
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // Z
};

uint8_t currentShape[4][4] = {0};

int currentType, currentX, currentY;
unsigned long lastFallTime = 0;

int fallSpeed = NORMAL_FALL_SPEED;

void drawBlock(int column, int row, uint16_t color)
{
  int px = BOARD_X + column * BLOCK_SIZE;
  int py = BOARD_Y + row * BLOCK_SIZE;

  if (color == TFT_BLACK)
  {
    tft.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, TFT_BLACK);
  }
  else
  {
    // Fill rect with the block color, leaving a 1-pixel border for the block outline
    tft.fillRect(px + 1, py + 1, BLOCK_SIZE - 2, BLOCK_SIZE - 2, color);

    // draw the block border
    tft.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, TFT_DARKGREY);
  }
}

void drawShape(bool erase)
{

  for (int row = 0; row < 4; row++)
  {
    for (int col = 0; col < 4; col++)
    {
      if (currentShape[row][col])
      {
        int bx = currentX + col;
        int by = currentY + row;

        // Only draw the block if it is within the board boundaries
        if (by >= 0 && by < BOARD_ROWS && bx >= 0 && bx < BOARD_COLS)
        {
          drawBlock(bx, by, erase ? TFT_BLACK : COLORS[currentType + 1]);
        }
      }
    }
  }
}

void spawnShape()
{
  currentType = random(0, 7);
  currentX = 3;
  currentY = 0;

  // Copy the shape into the currentShape array
  for (int row = 0; row < 4; row++)
  {
    for (int col = 0; col < 4; col++)
    {
      currentShape[row][col] = SHAPES[currentType][row][col];
    }
  }

  drawShape(false);
}

void resetGame()
{

  // reset the game board
  memset(board, 0, sizeof(board));

  fallSpeed = NORMAL_FALL_SPEED;
  lastFrameTime = 0;

  // Clear the screen with black color
  tft.fillScreen(TFT_BLACK);

  // Draw board
  tft.drawRect(BOARD_X - 2, BOARD_Y - 2, BOARD_COLS * BLOCK_SIZE + 4, BOARD_ROWS * BLOCK_SIZE + 4, TFT_WHITE); // Draw the board border

  spawnShape();
}

void setup()
{
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);

  // Initialize joystick pins
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(12); // Set the resolution for analog reads (0-4095)

  randomSeed(analogRead(36));

  tft.init();
  tft.setRotation(0);      // Set the display rotation if needed
  tft.invertDisplay(true); // Invert the display colors if needed

  resetGame();
}

void loop()
{
  // put your main code here, to run repeatedly:

  unsigned long nowFrameTime = micros();

  int currentSpeed = NORMAL_FALL_SPEED;

  unsigned long currentTimeMillis = 0;

  // Lock in 60 FPS (approximate frame timing)
  if (nowFrameTime - lastFrameTime >= DELTA_FRAME_TIME)
  {

    lastFrameTime = nowFrameTime;


    // Gravity / fall
    fallFrameCounter++;

    int targetFallFrame = DROP_DELAY_FRAMES; // Target number of frames to wait before allowing another fall

    currentSpeed = NORMAL_FALL_SPEED;

    if (fallFrameCounter >= targetFallFrame)
    {
      fallFrameCounter = 0;

      drawShape(true);
      currentY++;       // Move the shape down by one row each frame
      drawShape(false); // Redraw the shape at the new position
    }
  }
}
