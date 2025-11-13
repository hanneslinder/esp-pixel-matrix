#include "MatrixController.h"
#include "utils/utils.h"
#include <Fonts/Picopixel.h>

MatrixController* MatrixController::instance = nullptr;

MatrixController::MatrixController()
    : matrix(nullptr)
    , bgLayer(PANEL_WIDTH * PANEL_CHAIN, PANEL_HEIGHT, layer_draw_callback)
    , textLayer(PANEL_WIDTH * PANEL_CHAIN, PANEL_HEIGHT, layer_draw_callback)
    , gfx_compositor(layer_draw_callback)
{
  instance = this;
}

MatrixController::~MatrixController()
{
  if (matrix) {
    delete matrix;
  }
}

void MatrixController::begin()
{
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANEL_CHAIN,
      { R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN,
          OE_PIN, CLK_PIN });

  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_20M;
  mxconfig.min_refresh_rate = 60;

  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();

  // Start with minimum safe brightness (will be set from settings in main.cpp)
  // Values below 3 result in black screen
  matrix->setBrightness8(3);

  bgLayer.clear();
  textLayer.clear();

  gfx_compositor.Stack(bgLayer, textLayer);
}

void MatrixController::setBrightness(uint8_t brightness)
{
  if (matrix) {
    Serial.printf("MatrixController::setBrightness called with value: %d\n", brightness);
    matrix->setBrightness8(brightness);
    Serial.printf("setBrightness8(%d) completed\n", brightness);
  } else {
    Serial.println("ERROR: Matrix is null, cannot set brightness!");
  }
}

void MatrixController::clear()
{
  bgLayer.clear();
  textLayer.clear();
}

void MatrixController::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if (matrix) {
    matrix->drawPixel(x, y, color);
  }
}

void MatrixController::drawPixelRGB888(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b)
{
  if (matrix) {
    matrix->drawPixelRGB888(x, y, r, g, b);
  }
}

// Taken from https://github.com/mrcodetastic/GFX_Lite/blob/main/src/GFX_Layer.cpp
// Seems like there is a bug with setting the proper text color, therefore this is copied and
// adjusted here
void MatrixController::drawText(const char* buf, textPosition textPos, const GFXfont* f,
    uint16_t color, uint8_t size, int xAdjust, int yAdjust, int align)
{
  if (matrix) {
    int16_t x1, y1;
    uint16_t w, h;

    textLayer.setTextWrap(false);
    textLayer.setTextSize(size);

    if (f) {
      textLayer.setFont((GFXfont*)f);
    } else {
      textLayer.setFont();
    }

    textLayer.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);

    int16_t wstart = (PANEL_WIDTH - w) / 2 + xAdjust;

    if (align == 0) {
      wstart = 0 + xAdjust;
    } else if (align == 2) {
      wstart = PANEL_WIDTH - w + xAdjust;
    }

    if (!f) {
      if (textPos == TOP) {
        textLayer.setCursor(wstart, yAdjust); // top
      } else if (textPos == BOTTOM) {
        textLayer.setCursor(wstart, PANEL_HEIGHT - h + yAdjust);
      } else { // middle
        textLayer.setCursor(wstart, (PANEL_HEIGHT - h) / 2 + yAdjust); // top
      }
    } else {
      if (textPos == TOP) {
        textLayer.setCursor(wstart, h + yAdjust); // top
      } else if (textPos == BOTTOM) {
        textLayer.setCursor(wstart + 1, (PANEL_HEIGHT - 1) + yAdjust);
      } else { // middle
        textLayer.setCursor(wstart, ((PANEL_HEIGHT / 2) + (h / 2)) + yAdjust);
      }
    }

    textLayer.setTextColor(color);
    textLayer.println(buf);

    // textLayer.drawCentreText(buf, textPos, f, color, yAdjust);
  }
}

void MatrixController::layer_draw_callback(
    int16_t x, int16_t y, uint8_t r_data, uint8_t g_data, uint8_t b_data)
{
  if (instance) {
    instance->drawPixelRGB888(x, y, r_data, g_data, b_data);
  }
}

void MatrixController::render(uint8_t compositionMode)
{
  if (matrix) {
    switch (compositionMode) {
    case 0:
      getCompositor().Stack(getBackgroundLayer(), getTextLayer());
      break;
    case 1:
      getCompositor().Blend(getBackgroundLayer(), getTextLayer());
      break;
    case 2:
      getCompositor().Siloette(getBackgroundLayer(), getTextLayer());
      break;
    default:
      getCompositor().Stack(getBackgroundLayer(), getTextLayer());
      break;
    }
  }
}