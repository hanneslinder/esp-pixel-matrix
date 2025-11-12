#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class AsyncWebSocket;

namespace OTAUpdate {

// Initialize OTA update routes and progress callbacks.
// Must be called after WebSocket is created and before server.begin().
void init(AsyncWebServer& server, AsyncWebSocket& ws);

}
