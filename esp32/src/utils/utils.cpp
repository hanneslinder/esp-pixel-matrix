#include <Arduino.h>
#include <FastLED_Lite.h>
#include <sstream>

bool isStringEqual(const char* string1, const char* string2)
{
  return strcmp(string1, string2) == 0;
}

// Convert 5-6-5 color to 32bit hex value
String convert16BitTo32BitHexColor(uint16_t hexValue)
{
  unsigned r = (hexValue & 0xF800) >> 11;
  unsigned g = (hexValue & 0x07E0) >> 5;
  unsigned b = hexValue & 0x001F;

  r = (r * 255) / 31;
  g = (g * 255) / 63;
  b = (b * 255) / 31;

  char hex[8] = { 0 };
  sprintf(hex, "#%02X%02X%02X", r, g, b);
  return hex;
}

std::string convertRgbToHex(int r, int g, int b, bool with_head)
{
  std::stringstream ss;
  ss << std::hex << (r << 16 | g << 8 | b);

  std::string hex = ss.str();
  int str_length = hex.length();

  for (int i = 0; i < 6 - str_length; i++) {
    hex = "0" + hex;
  }

  if (with_head) {
    hex = "#" + hex;
  }

  return hex;
}

CRGB hexToCRGB(const std::string& hex)
{
  unsigned int r, g, b;
  if (hex[0] == '#') {
    std::stringstream ss;
    ss << std::hex << hex.substr(1); // Skip '#'
    unsigned int hexValue;
    ss >> hexValue;

    r = (hexValue >> 16) & 0xFF;
    g = (hexValue >> 8) & 0xFF;
    b = hexValue & 0xFF;
  } else {
    r = g = b = 40; // fallback
  }

  return CRGB(r, g, b);
}