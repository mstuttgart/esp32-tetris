/**
 * @file tetris.cpp
 * @brief Implementation of the Tetris game class.
 *
 * Full API documentation lives in tetris.h.
 */

#include "tetris.h"

Tetris::Tetris(TFT_eSPI &display)
    : tft(display),
      board{},
      currentShape{},
      currentType(0),
      currentX(0),
      currentY(0),
      score(0),
      gameOver(false),
      rotateReleased(true),
      lastFrameTime(0),
      moveFrameCounter(0),
      fallFrameCounter(0),
      gravityFrames(INITAL_GRAVITY_FRAMES)
{
}

void Tetris::begin()
{
  // Initialize joystick pins
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(12); // Set the resolution for analog reads (0-4095)

  randomSeed(analogRead(36));

  tft.init();
  tft.setRotation(0);      // Set the display rotation if needed
  tft.invertDisplay(true); // Invert the display colors if needed

  reset();
}

void Tetris::reset()
{
  // reset the game board
  memset(board, 0, sizeof(board));

  gameOver = false;
  score = 0;
  lastFrameTime = 0;
  moveFrameCounter = 0;
  fallFrameCounter = 0;
  gravityFrames = INITAL_GRAVITY_FRAMES;

  // Clear the screen with black color
  tft.fillScreen(TFT_BLACK);

  // Draw the board border
  tft.drawRect(BOARD_X - 2, BOARD_Y - 2, BOARD_COLS * BLOCK_SIZE + 4, BOARD_ROWS * BLOCK_SIZE + 4, TFT_WHITE);

  // Painel lateral
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(140, 20);
  tft.print("SCORE");

  updateScoreUI();

  spawnShape();
}

void Tetris::update()
{
  unsigned long nowFrameTime = micros();

  // Lock in 60 FPS (approximate frame timing)
  if (nowFrameTime - lastFrameTime < DELTA_FRAME_TIME)
  {
    return;
  }

  lastFrameTime = nowFrameTime;

  int joystickX = analogRead(JOYSTICK_X_PIN);
  int joystickY = analogRead(JOYSTICK_Y_PIN);

  bool buttonRotate = digitalRead(JOYSTICK_BUTTON_PIN) == LOW;

  if (gameOver)
  {
    if (buttonRotate)
    {
      delay(300);
      reset();
    }
    return;
  }

  handleHorizontalMove(joystickX);
  handleRotation(joystickY, buttonRotate);
  handleGravity(joystickY);
}

void Tetris::handleHorizontalMove(int joystickX)
{
  if (joystickX < JOYSTICK_DEADZONE_LOW || joystickX > JOYSTICK_DEADZONE_HIGH)
  {
    moveFrameCounter++;

    if (joystickX < JOYSTICK_DEADZONE_LOW && !checkCollision(currentX - 1, currentY, currentShape))
    {
      // Move left
      drawShape(true);
      currentX--;       // Move the shape left by one column
      drawShape(false); // Redraw the shape at the new position
    }
    else if (joystickX > JOYSTICK_DEADZONE_HIGH && !checkCollision(currentX + 1, currentY, currentShape))
    {
      // Move right
      drawShape(true);
      currentX++;       // Move the shape right by one column
      drawShape(false); // Redraw the shape at the new position
    }
  }
  else
  {
    moveFrameCounter = MOVE_DELAY_FRAMES; // Reset the move frame counter if joystick is in deadzone
  }
}

void Tetris::handleRotation(int joystickY, bool buttonRotate)
{
  bool rotateRequested = (joystickY < JOYSTICK_DEADZONE_LOW || buttonRotate);

  if (rotateRequested && rotateReleased)
  {
    rotateShape();
    rotateReleased = false;
  }
  else if (!rotateRequested)
  {
    rotateReleased = true;
  }
}

void Tetris::handleGravity(int joystickY)
{
  fallFrameCounter++;

  // Target number of frames to wait before allowing another fall
  int targetFallFrame = (joystickY > JOYSTICK_DEADZONE_HIGH) ? DROP_DELAY_FRAMES : gravityFrames;

  if (fallFrameCounter < targetFallFrame)
  {
    return;
  }

  fallFrameCounter = 0;

  if (!checkCollision(currentX, currentY + 1, currentShape))
  {
    drawShape(true);
    currentY++;       // Move the shape down by one row each frame
    drawShape(false); // Redraw the shape at the new position
  }
  else
  {
    lockShape();
  }
}

bool Tetris::checkCollision(int newX, int newY, const uint8_t shape[4][4]) const
{
  for (int row = 0; row < 4; row++)
  {
    for (int col = 0; col < 4; col++)
    {
      if (shape[row][col])
      {
        int bx = newX + col;
        int by = newY + row;

        if (by >= BOARD_ROWS || bx < 0 || bx >= BOARD_COLS || (by >= 0 && board[by][bx] != 0))
        {
          return true;
        }
      }
    }
  }
  return false;
}

void Tetris::spawnShape()
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

  if (checkCollision(currentX, currentY, currentShape))
  {
    triggerGameOver();
  }
}

void Tetris::lockShape()
{
  bool overflowTop = false;

  for (int row = 0; row < 4; row++)
  {
    for (int col = 0; col < 4; col++)
    {
      if (currentShape[row][col])
      {
        int bx = currentX + col;
        int by = currentY + row;

        if (by < 0)
        {
          overflowTop = true;
        }
        else
        {
          board[by][bx] = currentType + 1;
        }
      }
    }
  }

  if (overflowTop)
  {
    triggerGameOver();
    return;
  }

  clearLines();
  spawnShape();
}

void Tetris::rotateShape()
{
  uint8_t rotated[4][4] = {0};

  for (int row = 0; row < 4; row++)
  {
    for (int col = 0; col < 4; col++)
    {
      rotated[col][3 - row] = currentShape[row][col];
    }
  }

  if (!checkCollision(currentX, currentY, rotated))
  {
    drawShape(true);
    for (int row = 0; row < 4; row++)
    {
      for (int col = 0; col < 4; col++)
      {
        currentShape[row][col] = rotated[row][col];
      }
    }
    drawShape(false);
  }
}

void Tetris::clearLines()
{
  int linesCleared = 0;

  for (int r = BOARD_ROWS - 1; r >= 0; r--)
  {
    bool full = true;
    for (int c = 0; c < BOARD_COLS; c++)
    {
      if (board[r][c] == 0)
      {
        full = false;
        break;
      }
    }

    if (full)
    {
      linesCleared++;
      for (int row = r; row > 0; row--)
        for (int c = 0; c < BOARD_COLS; c++)
          board[row][c] = board[row - 1][c];
      for (int c = 0; c < BOARD_COLS; c++)
        board[0][c] = 0;
      r++;
    }
  }

  if (linesCleared > 0)
  {
    score += linesCleared * 100;

    int level = score / MAX_SCORE_INCREASE_LEVEL;

    gravityFrames = max(6, INITAL_GRAVITY_FRAMES - (level * 2));

    updateScoreUI();
    drawBoard();
  }
}

void Tetris::triggerGameOver()
{
  gameOver = true;
  tft.fillRect(20, 100, 200, 45, TFT_RED);
  tft.drawRect(20, 100, 200, 45, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(55, 115);
  tft.print("GAME OVER");
}

void Tetris::drawBlock(int column, int row, uint16_t color)
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

void Tetris::drawShape(bool erase)
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
        if (bx >= 0 && bx < BOARD_COLS && by >= 0 && by < BOARD_ROWS)
        {
          drawBlock(bx, by, erase ? TFT_BLACK : COLORS[currentType + 1]);
        }
      }
    }
  }
}

void Tetris::drawBoard()
{
  for (int r = 0; r < BOARD_ROWS; r++)
    for (int c = 0; c < BOARD_COLS; c++)
      drawBlock(c, r, COLORS[board[r][c]]);
}

void Tetris::updateScoreUI()
{
  tft.fillRect(140, 45, 95, 30, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(140, 50);
  tft.print(score);
}
