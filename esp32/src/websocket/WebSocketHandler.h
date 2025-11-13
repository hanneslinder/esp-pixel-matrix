#pragma once

#include "../types/CommonTypes.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

// Forward declarations
class MatrixController;
class TextDisplayHandler;

namespace WebSocketHandler {

// Initialize the WebSocket handler with required dependencies
void init(MatrixController* matrixCtrl, TextItem* textItems, AsyncWebSocket* websocket,
    char* socketBuffer, int* bufferIndex, const int bufferSize,
    TextDisplayHandler* textDisplayHandler);

// Main WebSocket message handler
void handleMessage(void* arg, uint8_t* data, size_t len);

// Event handlers
void onConnect(AsyncWebSocketClient* client);
void onDisconnect(AsyncWebSocketClient* client);
void onError(AsyncWebSocketClient* client);

// Utility functions
void sendPixels();
void sendState();

} // namespace WebSocketHandler
