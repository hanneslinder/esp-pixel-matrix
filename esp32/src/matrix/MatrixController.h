#pragma once

#include "../config/pins.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <GFX_Layer.hpp>

class MatrixController {
  public:
  MatrixController();
  ~MatrixController();

  void begin();
  void setBrightness(uint8_t brightness);
  void clear();
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void drawPixelRGB888(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b);
  void render(uint8_t compositionMode);

  void drawText(const char* buf, textPosition textPos, const GFXfont* f, uint16_t color,
      uint8_t size, int xadjust, int yadjust, int align);

  static void layer_draw_callback(
      int16_t x, int16_t y, uint8_t r_data, uint8_t g_data, uint8_t b_data);

  GFX_Layer& getBackgroundLayer() { return bgLayer; }
  GFX_Layer& getTextLayer() { return textLayer; }
  GFX_LayerCompositor& getCompositor() { return gfx_compositor; }

  private:
  MatrixPanel_I2S_DMA* matrix;
  GFX_Layer bgLayer;
  GFX_Layer textLayer;
  GFX_LayerCompositor gfx_compositor;
  static MatrixController* instance;
};