/**
 * @file tetris.h
 * @brief Declaration of the Tetris game class for ESP32 with a TFT display.
 */

#pragma once

#include "config.h"

/**
 * @class Tetris
 * @brief Encapsulates the full Tetris game: input, logic, rendering and state.
 *
 * The class owns the game state (board, falling piece, score, frame timing)
 * and renders to an externally owned TFT_eSPI display, injected through the
 * constructor. Typical usage:
 * @code
 * TFT_eSPI tft = TFT_eSPI();
 * Tetris game(tft);
 *
 * void setup() { game.begin(); }
 * void loop()  { game.update(); }
 * @endcode
 */
class Tetris
{
#ifdef UNIT_TEST
  friend struct TetrisTestAccess; ///< Grants unit tests access to internals.
#endif

public:
  /**
   * @brief Constructs the game bound to a display.
   * @param display Reference to an initialized-later TFT_eSPI instance.
   *                Must outlive this object.
   */
  explicit Tetris(TFT_eSPI &display);

  /**
   * @brief Initializes hardware and starts a new game.
   *
   * Configures the joystick pins, the ADC resolution, the random seed and
   * the TFT display, then calls reset(). Call once from setup().
   */
  void begin();

  /**
   * @brief Resets the game to its initial state.
   *
   * Clears the board, score and timing counters, redraws the static UI
   * (border and score panel) and spawns the first piece.
   */
  void reset();

  /**
   * @brief Runs one iteration of the game loop.
   *
   * Frame-locked at ::FPS: returns immediately if less than
   * ::DELTA_FRAME_TIME has elapsed since the last frame. Reads the joystick,
   * then applies horizontal movement, rotation and gravity. While in game
   * over, only waits for the button press to restart. Call from loop().
   */
  void update();

private:
  /** @name Input handling
   *  @{
   */
  /**
   * @brief Moves the piece left/right based on the joystick X axis.
   * @param joystickX Raw ADC reading of the X axis (0-4095).
   */
  void handleHorizontalMove(int joystickX);

  /**
   * @brief Rotates the piece on joystick-up or button press.
   *
   * Requires the input to be released between rotations (edge-triggered)
   * to avoid continuous spinning while held.
   *
   * @param joystickY Raw ADC reading of the Y axis (0-4095).
   * @param buttonRotate True if the joystick button is pressed.
   */
  void handleRotation(int joystickY, bool buttonRotate);

  /**
   * @brief Applies gravity and soft drop; locks the piece on landing.
   * @param joystickY Raw ADC reading of the Y axis (0-4095). Pushing down
   *                  (above ::JOYSTICK_DEADZONE_HIGH) speeds up the fall.
   */
  void handleGravity(int joystickY);
  /** @} */

  /** @name Game logic
   *  @{
   */
  /**
   * @brief Tests whether a shape collides at a given board position.
   * @param newX Candidate column of the shape's top-left corner.
   * @param newY Candidate row of the shape's top-left corner.
   * @param shape 4x4 bitmap of the shape to test.
   * @return True if the shape would overlap a locked block or leave the
   *         board on the sides/bottom; rows above the board are allowed.
   */
  bool checkCollision(int newX, int newY, const uint8_t shape[4][4]) const;

  /**
   * @brief Spawns a random piece at the top of the board.
   *
   * Triggers game over if the new piece immediately collides.
   */
  void spawnShape();

  /**
   * @brief Locks the current piece into the board.
   *
   * Triggers game over if any block lies above the board; otherwise clears
   * completed lines and spawns the next piece.
   */
  void lockShape();

  /**
   * @brief Rotates the current piece 90 degrees clockwise, if it fits.
   */
  void rotateShape();

  /**
   * @brief Removes completed lines, updates score, level and fall speed.
   *
   * Each cleared line awards 100 points; every ::MAX_SCORE_INCREASE_LEVEL
   * points the gravity interval shrinks (minimum of 6 frames).
   */
  void clearLines();

  /**
   * @brief Enters the game-over state and draws the game-over banner.
   */
  void triggerGameOver();
  /** @} */

  /** @name Rendering
   *  @{
   */
  /**
   * @brief Draws (or erases) a single block at a board cell.
   * @param column Board column (0 to ::BOARD_COLS - 1).
   * @param row Board row (0 to ::BOARD_ROWS - 1).
   * @param color Block color; TFT_BLACK erases the cell.
   */
  void drawBlock(int column, int row, uint16_t color);

  /**
   * @brief Draws or erases the current piece at its position.
   * @param erase True to erase (paint black), false to draw in the piece color.
   */
  void drawShape(bool erase);

  /**
   * @brief Redraws every cell of the board from the board array.
   */
  void drawBoard();

  /**
   * @brief Redraws the score value in the side panel.
   */
  void updateScoreUI();
  /** @} */

  TFT_eSPI &tft; ///< Display used for all rendering (externally owned).

  uint8_t board[BOARD_ROWS][BOARD_COLS]; ///< Playfield; 0 = empty, 1-7 = locked block color index.

  /** @name Current falling piece
   *  @{
   */
  uint8_t currentShape[4][4]; ///< 4x4 bitmap of the piece in its current rotation.
  int currentType;            ///< Piece type, index into ::SHAPES (0-6).
  int currentX;               ///< Column of the piece's top-left corner.
  int currentY;               ///< Row of the piece's top-left corner (may be negative at spawn).
  /** @} */

  int score;           ///< Current score.
  bool gameOver;       ///< True while in the game-over state.
  bool rotateReleased; ///< True when the rotate input has been released; prevents continuous spinning.

  /** @name Frame timing
   *  @{
   */
  unsigned long lastFrameTime; ///< micros() timestamp of the last processed frame.
  int moveFrameCounter;        ///< Counter for horizontal move delay frames.
  int fallFrameCounter;        ///< Counter for fall delay frames.
  int gravityFrames;           ///< Current frames between gravity steps (shrinks with level).
  /** @} */
};
