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
const int DEFAULT_BRIGHTNESS = 2;

// Reset Button Settings
const int RESET_SHORT_PRESS_TIME = 2000;

// WebSocket Settings
const int SOCKET_DATA_SIZE = 48000;