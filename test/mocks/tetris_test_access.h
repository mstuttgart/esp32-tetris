/**
 * @file tetris_test_access.h
 * @brief Test-only accessor for the private internals of the Tetris class.
 *
 * Declared as a friend of Tetris when UNIT_TEST is defined, so tests can
 * inspect and manipulate the game state without widening the public API.
 */

#pragma once

#include "tetris.h"

struct TetrisTestAccess
{
  static uint8_t (&board(Tetris &t))[BOARD_ROWS][BOARD_COLS] { return t.board; }
  static uint8_t (&shape(Tetris &t))[4][4] { return t.currentShape; }
  static int &currentType(Tetris &t) { return t.currentType; }
  static int &currentX(Tetris &t) { return t.currentX; }
  static int &currentY(Tetris &t) { return t.currentY; }
  static int &score(Tetris &t) { return t.score; }
  static bool &gameOver(Tetris &t) { return t.gameOver; }
  static int &gravityFrames(Tetris &t) { return t.gravityFrames; }

  static bool checkCollision(Tetris &t, int x, int y, const uint8_t s[4][4])
  {
    return t.checkCollision(x, y, s);
  }

  static void spawnShape(Tetris &t) { t.spawnShape(); }
  static void lockShape(Tetris &t) { t.lockShape(); }
  static void rotateShape(Tetris &t) { t.rotateShape(); }
  static void clearLines(Tetris &t) { t.clearLines(); }
};
