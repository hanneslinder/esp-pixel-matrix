#pragma once

#include <Arduino.h>

struct TextItem {
  char text[32];
  uint16_t color;
  int8_t offsetX;
  int8_t offsetY;
  uint8_t align;
  uint8_t size;
  uint8_t line;
  uint8_t font;
};
