/**
 * @file TFT_eSPI.h
 * @brief Mock of the TFT_eSPI display driver for native (host) unit tests.
 *
 * Implements the subset of the TFT_eSPI API used by the game as no-ops,
 * recording lightweight statistics that tests can assert on.
 */

#pragma once

#include <cstdint>

#define TFT_BLACK 0x0000
#define TFT_BLUE 0x001F
#define TFT_GREEN 0x07E0
#define TFT_CYAN 0x07FF
#define TFT_RED 0xF800
#define TFT_MAGENTA 0xF81F
#define TFT_YELLOW 0xFFE0
#define TFT_ORANGE 0xFDA0
#define TFT_DARKGREY 0x7BEF
#define TFT_WHITE 0xFFFF

class TFT_eSPI
{
public:
  int initCalls = 0;           ///< Number of init() calls.
  int fillScreenCalls = 0;     ///< Number of fillScreen() calls.
  int fillRectCalls = 0;       ///< Number of fillRect() calls.
  uint16_t lastFillRectColor = 0; ///< Color of the last fillRect() call.
  const char *lastText = nullptr; ///< Last string passed to print().
  long lastPrintedNumber = -1;    ///< Last number passed to print().

  void init() { initCalls++; }
  void setRotation(uint8_t) {}
  void invertDisplay(bool) {}
  void fillScreen(uint16_t) { fillScreenCalls++; }

  void fillRect(int32_t, int32_t, int32_t, int32_t, uint16_t color)
  {
    fillRectCalls++;
    lastFillRectColor = color;
  }

  void drawRect(int32_t, int32_t, int32_t, int32_t, uint16_t) {}
  void setTextColor(uint16_t) {}
  void setTextColor(uint16_t, uint16_t) {}
  void setTextSize(uint8_t) {}
  void setCursor(int16_t, int16_t) {}
  void print(const char *text) { lastText = text; }
  void print(int value) { lastPrintedNumber = value; }
};
