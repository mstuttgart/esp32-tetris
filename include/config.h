/**
 * @file config.h
 * @brief Compile-time configuration for the ESP32 Tetris game.
 *
 * Holds all game constants: frame timing, joystick pins and deadzone,
 * board geometry, piece colors and tetromino shape definitions.
 */

#pragma once // Evita inclusão duplicada (substitui os antigos #ifndef / #define)

#include <Arduino.h>
#include <TFT_eSPI.h>

/** @name Frame timing
 *  @{
 */
const int FPS = 60;                                     ///< Frames per second for the game loop.
const unsigned long DELTA_FRAME_TIME = 1000000UL / FPS; ///< Duration of one frame in microseconds (~16666 us).

const int MOVE_DELAY_FRAMES = 7;      ///< Frames to wait before allowing another horizontal move.
const int DROP_DELAY_FRAMES = 3;      ///< Frames to wait between falls during a soft drop.
const int INITAL_GRAVITY_FRAMES = 30; ///< Initial frames between gravity steps (0.5 s at 60 FPS).
/** @} */

const int SERIAL_BAUD_RATE = 115200; ///< Serial communication baud rate.

/** @name Joystick
 *  @{
 */
const int JOYSTICK_X_PIN = 34;      ///< Analog pin for the joystick X axis.
const int JOYSTICK_Y_PIN = 35;      ///< Analog pin for the joystick Y axis.
const int JOYSTICK_BUTTON_PIN = 32; ///< Digital pin for the joystick push button (active low).

const int JOYSTICK_DEADZONE_LOW = 1000;  ///< Readings below this value count as a negative-axis input.
const int JOYSTICK_DEADZONE_HIGH = 3000; ///< Readings above this value count as a positive-axis input.
/** @} */

/** @name Board geometry
 *  @{
 */
const int BOARD_WIDTH = 240;  ///< Display width in pixels.
const int BOARD_HEIGHT = 240; ///< Display height in pixels.
const int BOARD_COLS = 10;    ///< Number of columns in the playfield.
const int BOARD_ROWS = 20;    ///< Number of rows in the playfield.
const int BLOCK_SIZE = 11;    ///< Size of a single block in pixels.
const int BOARD_X = 15;       ///< X offset of the playfield on the display, in pixels.
const int BOARD_Y = 10;       ///< Y offset of the playfield on the display, in pixels.
/** @} */

const int MAX_SCORE_INCREASE_LEVEL = 500; ///< Score points required to advance one level.

/**
 * @brief Block colors indexed by cell value.
 *
 * Index 0 (TFT_BLACK) represents an empty cell; indexes 1-7 map to the
 * tetromino types in ::SHAPES (shape type + 1).
 */
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

/**
 * @brief Tetromino shape definitions in their spawn orientation.
 *
 * Each shape is a 4x4 bitmap where 1 marks a filled cell.
 * Order: I, J, L, O, S, T, Z.
 */
const uint8_t SHAPES[7][4][4] = {
    {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // J
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // L
    {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // O
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // S
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // T
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}  // Z
};
