#pragma once

// WiFi Portal Configuration
const char* ntpServer = "at.pool.ntp.org";
const char* portalIP = "10.0.1.1";

// Time and Locale Settings
// https://ftp.fau.de/aminet/util/time/tzinfo.txt
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* locale = "en_US.UTF-8";
const char* hostname = "pixelclock";

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