#pragma once

// This header declares fixed-width and size types itself, and is included first
// by other headers (ConfigManager.h), so it must not depend on Arduino.h having
// been pulled in beforehand. Keep both includes: <cstddef> for size_t and
// <cstdint> for uint16_t/uint8_t.
#include <cstddef>
#include <cstdint>

// WiFi Portal Configuration
extern const char* ntpServer;
extern const char* portalIP;

// Time and Locale Settings
// https://ftp.fau.de/aminet/util/time/tzinfo.txt
extern const char* timezone;
extern const char* locale;
extern const char* hostname;

// Display Settings
const int MAX_BRIGHTNESS = 15;
const int MIN_BRIGHTNESS = 3;
const int DEFAULT_BRIGHTNESS = 3;

// Quiet Hours Settings
// Stored as "HH:MM" plus a terminator. Quiet hours are disabled when either
// bound is empty.
const size_t QUIET_HOURS_TIME_LENGTH = 6;
// Brightness used while quiet hours are active. The panel renders black below
// MIN_BRIGHTNESS, so the sleep icon needs at least this much. This is global to
// the panel, so the icon itself cannot be dimmed further - see the colour below.
const int QUIET_HOURS_BRIGHTNESS = MIN_BRIGHTNESS;
// Colour of the sleep icon, RGB565. A dim grey rather than pure white: the
// panel brightness is already at its floor, so this is the only remaining way
// to make the icon less noticeable at night. 0xFFFF would be white.
const uint16_t QUIET_HOURS_ICON_COLOR = 0x4208;

// Reset Button Settings
const int RESET_SHORT_PRESS_TIME = 2000;

// WebSocket Settings
// Buffer size for WebSocket messages (must accommodate full image data + JSON overhead)
// 64x32 pixels * 6 bytes per pixel (hex color) + JSON structure ≈ 12KB + overhead
const int SOCKET_DATA_SIZE = 32768; // 32KB should be sufficient for 64x32 images