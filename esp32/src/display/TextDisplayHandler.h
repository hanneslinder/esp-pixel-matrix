#ifndef TEXT_DISPLAY_HANDLER_H
#define TEXT_DISPLAY_HANDLER_H

#include <Arduino.h>
#include <gfxfont.h>
#include <time.h>

#include "../matrix/MatrixController.h"
#include "../types/CommonTypes.h"

class TextDisplayHandler {
  public:
  TextDisplayHandler(MatrixController& matrix, TextItem* textContent, size_t textContentSize);

  void setLocale(const char* locale);
  void renderText();

  TextItem* getTextContent();
  size_t getTextContentSize() const;
  const char* getCurrentLocale() const;

  private:
  void renderTextItem(const char* text, TextItem& item);

  MatrixController& _matrix;
  TextItem* _textContent;
  size_t _textContentSize;
  char _currentLocale[32];
};

#endif // TEXT_DISPLAY_HANDLER_H
