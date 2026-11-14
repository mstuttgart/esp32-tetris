/**
 * @file Arduino.h
 * @brief Mock of the Arduino core for native (host) unit tests.
 *
 * Provides controllable stand-ins for the hardware functions used by the
 * game: analog/digital reads, timing and random numbers. Tests manipulate
 * the mock* variables to simulate joystick input and the passage of time.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>

#define INPUT_PULLUP 0x05
#define LOW 0x00
#define HIGH 0x01

inline unsigned long mockMicros = 0;  ///< Value returned by micros().
inline int mockAnalog[64] = {0};      ///< Values returned by analogRead(), per pin.
inline int mockDigital[64] = {0};     ///< Values returned by digitalRead(), per pin.
inline long mockRandomResult = 0;     ///< Value returned by random() (clamped to the range).

/// Resets all mock inputs to a neutral state: joystick centered, button released, time zeroed.
inline void mockResetInputs()
{
  for (int i = 0; i < 64; i++)
  {
    mockAnalog[i] = 2048; // joystick center (inside the deadzone)
    mockDigital[i] = HIGH; // button released (active low)
  }
  mockMicros = 0;
  mockRandomResult = 0;
}

inline void pinMode(uint8_t, uint8_t) {}
inline void analogReadResolution(uint8_t) {}
inline void randomSeed(unsigned long) {}
inline int analogRead(uint8_t pin) { return mockAnalog[pin]; }
inline int digitalRead(uint8_t pin) { return mockDigital[pin]; }
inline unsigned long micros() { return mockMicros; }
inline void delay(unsigned long ms) { mockMicros += ms * 1000UL; }

inline long random(long minV, long maxV)
{
  long v = mockRandomResult;
  if (v < minV)
    v = minV;
  if (v >= maxV)
    v = maxV - 1;
  return v;
}

inline int max(int a, int b) { return a > b ? a : b; }
