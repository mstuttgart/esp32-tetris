/**
 * @file test_game_flow.cpp
 * @brief Integration tests driving the full game loop (Tetris::update)
 *        through the mocked joystick, button and clock.
 */

#include <unity.h>

#include "tetris.h"
#include "tetris_test_access.h"

namespace
{
constexpr int T_PIECE = 5; ///< Index of the T piece in SHAPES.

TFT_eSPI tft;
Tetris *game = nullptr;

/// Advances the mock clock by one frame and runs one game-loop iteration.
void runFrames(int frames)
{
  for (int i = 0; i < frames; i++)
  {
    mockMicros += DELTA_FRAME_TIME;
    game->update();
  }
}
} // namespace

void setUp()
{
  mockResetInputs();
  mockRandomResult = T_PIECE; // deterministic spawns: always the T piece

  delete game;
  game = new Tetris(tft);
  game->begin();
}

void tearDown()
{
  delete game;
  game = nullptr;
}

void test_piece_falls_after_gravity_frames()
{
  runFrames(INITAL_GRAVITY_FRAMES - 1);
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::currentY(*game));

  runFrames(1);
  TEST_ASSERT_EQUAL_INT(1, TetrisTestAccess::currentY(*game));
}

void test_joystick_moves_piece_horizontally()
{
  mockAnalog[JOYSTICK_X_PIN] = 0; // push left
  runFrames(2);
  TEST_ASSERT_EQUAL_INT(1, TetrisTestAccess::currentX(*game));

  mockAnalog[JOYSTICK_X_PIN] = 4095; // push right
  runFrames(3);
  TEST_ASSERT_EQUAL_INT(4, TetrisTestAccess::currentX(*game));
}

void test_piece_stops_at_left_wall()
{
  mockAnalog[JOYSTICK_X_PIN] = 0; // hold left
  runFrames(10);
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::currentX(*game));
}

void test_soft_drop_accelerates_fall()
{
  mockAnalog[JOYSTICK_Y_PIN] = 4095; // push down
  runFrames(DROP_DELAY_FRAMES);
  TEST_ASSERT_EQUAL_INT(1, TetrisTestAccess::currentY(*game));

  runFrames(DROP_DELAY_FRAMES);
  TEST_ASSERT_EQUAL_INT(2, TetrisTestAccess::currentY(*game));
}

void test_rotation_is_edge_triggered()
{
  mockDigital[JOYSTICK_BUTTON_PIN] = LOW; // press and hold the button
  runFrames(1);

  uint8_t afterFirstRotation[4][4];
  memcpy(afterFirstRotation, TetrisTestAccess::shape(*game), sizeof(afterFirstRotation));

  // Holding the button must not rotate again
  runFrames(4);
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      TEST_ASSERT_EQUAL_UINT8(afterFirstRotation[r][c], TetrisTestAccess::shape(*game)[r][c]);

  // Release and press again -> rotates a second time
  mockDigital[JOYSTICK_BUTTON_PIN] = HIGH;
  runFrames(1);
  mockDigital[JOYSTICK_BUTTON_PIN] = LOW;
  runFrames(1);

  bool changed = false;
  for (int r = 0; r < 4 && !changed; r++)
    for (int c = 0; c < 4 && !changed; c++)
      changed = afterFirstRotation[r][c] != TetrisTestAccess::shape(*game)[r][c];
  TEST_ASSERT_TRUE(changed);
}

void test_full_drop_locks_piece_and_spawns_next()
{
  mockAnalog[JOYSTICK_Y_PIN] = 4095; // hold soft drop

  // T spawns at y=0 and rests at y=18: 18 fall steps + 1 locking step,
  // each taking DROP_DELAY_FRAMES frames.
  runFrames(DROP_DELAY_FRAMES * 19);

  auto &board = TetrisTestAccess::board(*game);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[18][4]);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[19][3]);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[19][4]);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[19][5]);

  // Next piece already falling from the top
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::currentY(*game));
  TEST_ASSERT_FALSE(TetrisTestAccess::gameOver(*game));
}

void test_stacking_to_the_top_triggers_game_over()
{
  mockAnalog[JOYSTICK_Y_PIN] = 4095; // hold soft drop forever

  // Pieces stack on top of each other in the same column; eventually the
  // spawn position is occupied and the game must end.
  runFrames(DROP_DELAY_FRAMES * 19 * 12);

  TEST_ASSERT_TRUE(TetrisTestAccess::gameOver(*game));
}

void test_button_restarts_after_game_over()
{
  TetrisTestAccess::gameOver(*game) = true;
  TetrisTestAccess::score(*game) = 700;
  TetrisTestAccess::board(*game)[19][0] = 1;

  mockDigital[JOYSTICK_BUTTON_PIN] = LOW; // press restart
  runFrames(1);

  TEST_ASSERT_FALSE(TetrisTestAccess::gameOver(*game));
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::score(*game));
  TEST_ASSERT_EQUAL_UINT8(0, TetrisTestAccess::board(*game)[19][0]);
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::currentY(*game)); // fresh piece spawned
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_piece_falls_after_gravity_frames);
  RUN_TEST(test_joystick_moves_piece_horizontally);
  RUN_TEST(test_piece_stops_at_left_wall);
  RUN_TEST(test_soft_drop_accelerates_fall);
  RUN_TEST(test_rotation_is_edge_triggered);
  RUN_TEST(test_full_drop_locks_piece_and_spawns_next);
  RUN_TEST(test_stacking_to_the_top_triggers_game_over);
  RUN_TEST(test_button_restarts_after_game_over);
  return UNITY_END();
}
