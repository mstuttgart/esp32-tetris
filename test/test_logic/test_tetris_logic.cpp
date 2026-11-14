/**
 * @file test_tetris_logic.cpp
 * @brief Unit tests for the Tetris game logic (collision, rotation,
 *        line clearing, locking and spawning), run on the native platform.
 */

#include <unity.h>

#include "tetris.h"
#include "tetris_test_access.h"

namespace
{
constexpr int T_PIECE = 5; ///< Index of the T piece in SHAPES.

TFT_eSPI tft;
Tetris *game = nullptr;
} // namespace

void setUp()
{
  mockResetInputs();
  mockRandomResult = T_PIECE; // deterministic spawns: always the T piece

  delete game;
  game = new Tetris(tft);
  game->reset();
}

void tearDown()
{
  delete game;
  game = nullptr;
}

// --- Spawning -------------------------------------------------------------

void test_spawn_initial_state()
{
  TEST_ASSERT_EQUAL_INT(T_PIECE, TetrisTestAccess::currentType(*game));
  TEST_ASSERT_EQUAL_INT(3, TetrisTestAccess::currentX(*game));
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::currentY(*game));
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::score(*game));
  TEST_ASSERT_FALSE(TetrisTestAccess::gameOver(*game));

  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      TEST_ASSERT_EQUAL_UINT8(SHAPES[T_PIECE][r][c], TetrisTestAccess::shape(*game)[r][c]);
}

void test_spawn_on_occupied_board_triggers_game_over()
{
  auto &board = TetrisTestAccess::board(*game);
  for (int c = 0; c < BOARD_COLS; c++)
  {
    board[0][c] = 1;
    board[1][c] = 1;
  }

  TetrisTestAccess::spawnShape(*game);

  TEST_ASSERT_TRUE(TetrisTestAccess::gameOver(*game));
}

// --- Collision detection ---------------------------------------------------

void test_collision_with_walls_and_floor()
{
  auto &shape = TetrisTestAccess::shape(*game); // T: cells in cols 0-2, rows 0-1

  TEST_ASSERT_TRUE(TetrisTestAccess::checkCollision(*game, -1, 0, shape));  // left wall
  TEST_ASSERT_TRUE(TetrisTestAccess::checkCollision(*game, 8, 0, shape));   // right wall
  TEST_ASSERT_TRUE(TetrisTestAccess::checkCollision(*game, 3, 19, shape));  // below floor
  TEST_ASSERT_FALSE(TetrisTestAccess::checkCollision(*game, 0, 0, shape));  // leftmost valid
  TEST_ASSERT_FALSE(TetrisTestAccess::checkCollision(*game, 7, 0, shape));  // rightmost valid
  TEST_ASSERT_FALSE(TetrisTestAccess::checkCollision(*game, 3, 18, shape)); // resting on floor
}

void test_collision_with_locked_blocks()
{
  auto &shape = TetrisTestAccess::shape(*game);
  TetrisTestAccess::board(*game)[11][4] = 1; // locked block below the piece

  // T at (3,10) occupies rows 10-11, col 4 on both -> overlaps the block
  TEST_ASSERT_TRUE(TetrisTestAccess::checkCollision(*game, 3, 10, shape));
  // One row higher there is no overlap
  TEST_ASSERT_FALSE(TetrisTestAccess::checkCollision(*game, 3, 9, shape));
}

// --- Rotation ---------------------------------------------------------------

void test_rotate_t_piece_clockwise()
{
  TetrisTestAccess::rotateShape(*game);

  const uint8_t expected[4][4] = {
      {0, 0, 1, 0},
      {0, 0, 1, 1},
      {0, 0, 1, 0},
      {0, 0, 0, 0}};

  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      TEST_ASSERT_EQUAL_UINT8(expected[r][c], TetrisTestAccess::shape(*game)[r][c]);
}

void test_rotation_blocked_by_locked_block_keeps_shape()
{
  // The rotated T would occupy (row 1, col 3) -> board cell (1, 6); block it.
  TetrisTestAccess::board(*game)[1][6] = 1;

  TetrisTestAccess::rotateShape(*game);

  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      TEST_ASSERT_EQUAL_UINT8(SHAPES[T_PIECE][r][c], TetrisTestAccess::shape(*game)[r][c]);
}

// --- Line clearing ----------------------------------------------------------

void test_clear_single_line_scores_100()
{
  auto &board = TetrisTestAccess::board(*game);
  for (int c = 0; c < BOARD_COLS; c++)
    board[BOARD_ROWS - 1][c] = 1;

  TetrisTestAccess::clearLines(*game);

  TEST_ASSERT_EQUAL_INT(100, TetrisTestAccess::score(*game));
  for (int c = 0; c < BOARD_COLS; c++)
    TEST_ASSERT_EQUAL_UINT8(0, board[BOARD_ROWS - 1][c]);
}

void test_clear_line_shifts_rows_down()
{
  auto &board = TetrisTestAccess::board(*game);
  board[BOARD_ROWS - 2][0] = 3; // marker block above the full line
  for (int c = 0; c < BOARD_COLS; c++)
    board[BOARD_ROWS - 1][c] = 1;

  TetrisTestAccess::clearLines(*game);

  TEST_ASSERT_EQUAL_UINT8(3, board[BOARD_ROWS - 1][0]); // marker fell one row
  TEST_ASSERT_EQUAL_UINT8(0, board[BOARD_ROWS - 2][0]);
}

void test_level_up_speeds_up_gravity()
{
  TetrisTestAccess::score(*game) = 400; // 200 more points -> level 1

  auto &board = TetrisTestAccess::board(*game);
  for (int c = 0; c < BOARD_COLS; c++)
  {
    board[BOARD_ROWS - 1][c] = 1;
    board[BOARD_ROWS - 2][c] = 1;
  }

  TetrisTestAccess::clearLines(*game);

  TEST_ASSERT_EQUAL_INT(600, TetrisTestAccess::score(*game));
  TEST_ASSERT_EQUAL_INT(INITAL_GRAVITY_FRAMES - 2, TetrisTestAccess::gravityFrames(*game));
}

// --- Locking ----------------------------------------------------------------

void test_lock_shape_writes_board_and_spawns_next()
{
  TetrisTestAccess::currentY(*game) = 18; // T resting on the floor

  TetrisTestAccess::lockShape(*game);

  auto &board = TetrisTestAccess::board(*game);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[18][4]);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[19][3]);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[19][4]);
  TEST_ASSERT_EQUAL_UINT8(T_PIECE + 1, board[19][5]);

  // A new piece was spawned at the top
  TEST_ASSERT_EQUAL_INT(0, TetrisTestAccess::currentY(*game));
  TEST_ASSERT_FALSE(TetrisTestAccess::gameOver(*game));
}

void test_lock_above_top_triggers_game_over()
{
  TetrisTestAccess::currentY(*game) = -1; // T top row sticks out of the board

  TetrisTestAccess::lockShape(*game);

  TEST_ASSERT_TRUE(TetrisTestAccess::gameOver(*game));
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_spawn_initial_state);
  RUN_TEST(test_spawn_on_occupied_board_triggers_game_over);
  RUN_TEST(test_collision_with_walls_and_floor);
  RUN_TEST(test_collision_with_locked_blocks);
  RUN_TEST(test_rotate_t_piece_clockwise);
  RUN_TEST(test_rotation_blocked_by_locked_block_keeps_shape);
  RUN_TEST(test_clear_single_line_scores_100);
  RUN_TEST(test_clear_line_shifts_rows_down);
  RUN_TEST(test_level_up_speeds_up_gravity);
  RUN_TEST(test_lock_shape_writes_board_and_spawns_next);
  RUN_TEST(test_lock_above_top_triggers_game_over);
  return UNITY_END();
}
