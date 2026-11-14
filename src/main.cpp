/**
 * @file main.cpp
 * @brief Application entry point: wires the Tetris game to the Arduino runtime.
 */

#include <Arduino.h>
#include <SPI.h>

#include "tetris.h"

TFT_eSPI tft = TFT_eSPI(); ///< Display driver instance shared with the game.
Tetris game(tft);          ///< Game instance rendering to @ref tft.

/**
 * @brief Arduino setup hook: initializes serial and the game.
 */
void setup()
{
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);

  game.begin();
}

/**
 * @brief Arduino loop hook: runs one game-loop iteration per call.
 */
void loop()
{
  game.update();
}
