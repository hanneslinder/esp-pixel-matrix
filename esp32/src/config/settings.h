#pragma once

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

// Reset Button Settings
const int RESET_SHORT_PRESS_TIME = 2000;

// WebSocket Settings
// Buffer size for WebSocket messages (must accommodate full image data + JSON overhead)
// 64x32 pixels * 6 bytes per pixel (hex color) + JSON structure ≈ 12KB + overhead
const int SOCKET_DATA_SIZE = 32768; // 32KB should be sufficient for 64x32 images