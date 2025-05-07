#include "MatrixController.h"
#include <Fonts/Picopixel.h>

MatrixController* MatrixController::instance = nullptr;

MatrixController::MatrixController()
    : matrix(nullptr)
    , bgLayer(PANEL_RES_X * PANEL_CHAIN, PANEL_RES_Y, layer_draw_callback)
    , textLayer(PANEL_RES_X * PANEL_CHAIN, PANEL_RES_Y, layer_draw_callback)
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
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN,
      { R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN,
          OE_PIN, CLK_PIN });

  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();
  matrix->setBrightness8(10);

  bgLayer.clear();
  textLayer.clear();

  textLayer.drawCentreText("starting up...", MIDDLE, &Picopixel, NULL, 0);

  gfx_compositor.Stack(bgLayer, textLayer);
}

void MatrixController::setBrightness(uint8_t brightness)
{
  if (matrix) {
    matrix->setBrightness8(brightness);
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

void MatrixController::drawText(const char* buf, textPosition textPos, const GFXfont* f,
    uint16_t color, uint8_t size, int xAdjust, int yAdjust, int align)
{
  if (matrix) {

    // Set custom font
    if (f) {
      textLayer.setFont((GFXfont*)f);
    } else {
      textLayer.setFont();
    }

    int16_t xOne, yOne;
    uint16_t w, h;

    textLayer.setTextWrap(false);
    textLayer.setTextColor(color);
    textLayer.setTextSize(size);

    matrix->getTextBounds(buf, 0, 0, &xOne, &yOne, &w, &h);

    int wstart = (PANEL_RES_X - w) / 2 + xAdjust;

    if (align == 0) { // left align
      wstart = 0 + xAdjust;
    } else if (align == 2) { // righ align
      wstart = PANEL_RES_X - w + xAdjust;
    }

    if (!f) {
      if (textPos == TOP) {
        textLayer.setCursor(wstart, 0); // top
      } else if (textPos == BOTTOM) {
        textLayer.setCursor(wstart, PANEL_RES_Y - h + yAdjust);
      } else { // middle
        textLayer.setCursor(wstart, (PANEL_RES_Y - h) / 2 + yAdjust); // top
      }
    } else {
      if (textPos == TOP) {
        textLayer.setCursor(wstart, h + yAdjust); // top
      } else if (textPos == BOTTOM) {
        textLayer.setCursor(wstart + 1, (PANEL_RES_Y - 1) + yAdjust);
      } else { // middle
        textLayer.setCursor(wstart, ((PANEL_RES_Y / 2) + (h / 2)) + yAdjust);
      }
    }

    textLayer.println(buf);
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