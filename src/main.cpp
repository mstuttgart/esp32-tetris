#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// Serial communication baud rate
const int SERIAL_BAUD_RATE = 115200;

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
const int board[BOARD_ROWS][BOARD_COLS] = {0};




void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);

  // Initialize joystick pins
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);

  tft.init();
  tft.setRotation(0); // Set the display rotation if needed
  tft.invertDisplay(true); // Invert the display colors if needed
  tft.fillScreen(TFT_BLACK); // Clear the screen with black color

}

void loop() {
  // put your main code here, to run repeatedly:
}

