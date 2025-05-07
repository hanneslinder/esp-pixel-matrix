#include <Arduino.h>

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