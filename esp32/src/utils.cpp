#include <Arduino.h>

bool isStringEqual(const char* string1, const char* string2) {
	return strcmp(string1, string2) == 0;
}