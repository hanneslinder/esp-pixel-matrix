#include "TextDisplayHandler.h"
#include <Fonts/Picopixel.h>

TextDisplayHandler::TextDisplayHandler(
    MatrixController& matrix, TextItem* textContent, size_t textContentSize)
    : _matrix(matrix)
    , _textContent(textContent)
    , _textContentSize(textContentSize)
{
  strlcpy(_currentLocale, "en_US.UTF-8", sizeof(_currentLocale));
}

void TextDisplayHandler::setLocale(const char* locale)
{
  strlcpy(_currentLocale, locale, sizeof(_currentLocale));

  if (setlocale(LC_ALL, _currentLocale) == NULL) {
    Serial.printf("Unable to set locale %s\n", _currentLocale);
    return;
  }

  std::setlocale(LC_TIME, _currentLocale);
  std::setlocale(LC_NUMERIC, _currentLocale);

  Serial.printf("Set locale to %s\n", _currentLocale);
}

void TextDisplayHandler::renderTextItem(const char* text, TextItem& item)
{
  textPosition pos = static_cast<textPosition>(item.line);

  if (item.font == 0) {
    _matrix.drawText(
        text, pos, NULL, item.color, item.size, item.offsetX, item.offsetY, item.align);
  } else {
    _matrix.drawText(
        text, pos, &Picopixel, item.color, item.size, item.offsetX, item.offsetY, item.align);
  }
}

void TextDisplayHandler::renderText()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  _matrix.getTextLayer().clear();

  for (size_t i = 0; i < _textContentSize; i++) {
    if (_textContent[i].text[0] == '\0') {
      // Skip empty text items
      continue;
    }

    char parsedDate[32];
    strftime(parsedDate, 32, _textContent[i].text, &timeinfo);
    renderTextItem(parsedDate, _textContent[i]);
  }
}

TextItem* TextDisplayHandler::getTextContent() { return _textContent; }

size_t TextDisplayHandler::getTextContentSize() const { return _textContentSize; }

const char* TextDisplayHandler::getCurrentLocale() const { return _currentLocale; }
