#include "SPIFFS.h"
#include "time.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Fonts/Picopixel.h>
#include <GFX_Layer.hpp>
#include <HTTPClient.h>
#include <MycilaESPConnect.h>
#include <Update.h>
#include <WiFi.h>
#include <Wire.h>
#include <sstream>

#include "config/ConfigManager.h"
#include "config/pins.h"
#include "config/secrets.h"
#include "config/settings.h"
#include "data/CustomDataHandler.h"
#include "display/TextDisplayHandler.h"
#include "input/ResetButtonHandler.h"
#include "matrix/MatrixController.h"
#include "ota/OTAUpdateHandler.h"
#include "server/WebServerHandler.h"
#include "types/CommonTypes.h"
#include "utils/utils.h"
#include "websocket/WebSocketHandler.h"
#include "wifi/WiFiConnectionHandler.h"

#define U_PART U_SPIFFS

// Get singleton instance of ConfigManager
ConfigManager& config = ConfigManager::getInstance();

// Moved OTA progress state into OTAUpdate module

MatrixController matrix;

bool startupFinished = false;

// clock options - to be migrated to state manager
boolean showText = true;
boolean lastshowText = false;
boolean forceUpdateTime = false;

// Mutable copies for runtime configuration
char currentTimezone[64];

TextItem textContent[5]
    = { { "%H:%M", 0xFFFF, 1, -3, 1, 2, 1 }, { "%d.%b", 0xFFFF, 3, -1, 1, 1, 2 } };

char* socketData;
int currSocketBufferIndex = 0;

// custom data - kept for WebSocketHandler access
int customDataUpdateInterval = -1;
boolean customDataEnabled = false;
char customDataServer[128] = "";

static unsigned long lastHeapCheck = 0;
static size_t minFreeHeap = SIZE_MAX;

// Initialize handlers and server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiConnectionHandler wifiHandler(server);
ResetButtonHandler resetButton(RESET_PIN, RESET_SHORT_PRESS_TIME);
TextDisplayHandler textDisplay(matrix, textContent, 5);
WebServerHandler webServer(server, ws);
CustomDataHandler customData;

void initMatrix() { matrix.begin(); }

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg,
    uint8_t* data, size_t len)
{
  switch (type) {
  case WS_EVT_CONNECT:
    WebSocketHandler::onConnect(client);
    break;
  case WS_EVT_DISCONNECT:
    WebSocketHandler::onDisconnect(client);
    break;
  case WS_EVT_DATA:
    WebSocketHandler::handleMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
    break;
  case WS_EVT_ERROR:
    WebSocketHandler::onError(client);
    break;
  }
}

void resetWifi() { wifiHandler.reset(); }

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  ws.enable(true);

  WebSocketHandler::init(&matrix, textContent, &ws, socketData, &currSocketBufferIndex,
      SOCKET_DATA_SIZE, &textDisplay, &customData);
}

void checkHeapAndLog()
{
  size_t freeHeap = ESP.getFreeHeap();
  size_t heapSize = ESP.getHeapSize();
  size_t minHeap = ESP.getMinFreeHeap();

  if (freeHeap < minFreeHeap) {
    minFreeHeap = freeHeap;
  }

  Serial.println("=== System Status ===");
  Serial.printf("Free Heap: %u bytes (%.1f%%)\n", freeHeap, (freeHeap * 100.0) / heapSize);
  Serial.printf("Min Free Heap: %u bytes\n", minHeap);
  Serial.printf("Heap Size: %u bytes\n", heapSize);
  Serial.printf("WiFi Status: %s (RSSI: %d dBm)\n",
      wifiHandler.isConnected() ? "Connected" : "Disconnected", wifiHandler.getRSSI());
  Serial.printf("WebSocket Clients: %u\n", ws.count());
  Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
  Serial.println("====================");

  // Warning if heap is getting low
  if (freeHeap < 30000) {
    Serial.println("WARNING: Low heap memory! System may become unstable.");
  }
}

void setup()
{
  Serial.begin(115200);
  SPIFFS.begin();

  // Initialize ConfigManager first
  config.begin();

  // Initialize text display with locale from config
  textDisplay.setLocale(config.getLocale());

  socketData = (char*)malloc(SOCKET_DATA_SIZE * sizeof(char));

  // Initialize reset button
  resetButton.begin();
  resetButton.onLongPress([]() { resetWifi(); });

  // Copy timezone from config for runtime use
  strlcpy(currentTimezone, config.getTimezone(), sizeof(currentTimezone));

  // Initialize WiFi
  wifiHandler.begin(useCaptivePortal, ssid, password);

  // Initialize matrix
  initMatrix();

  // Apply brightness from config
  matrix.setBrightness(config.getBrightness());
  Serial.printf("Set initial brightness to %d\n", config.getBrightness());

  // Initialize WebSocket and Web Server
  initWebSocket();
  webServer.begin();

  // Configure timezone and NTP
  configTzTime(config.getTimezone(), config.getNtpServer());
  Serial.printf("Configured NTP: %s, Timezone: %s\n", config.getNtpServer(), config.getTimezone());
}

void loop()
{
  wifiHandler.loop();
  resetButton.update();

  if (startupFinished == false) {
    const String ip = wifiHandler.getIPAddress();

    matrix.drawText(ip.c_str(), MIDDLE, &Picopixel, 0xFFFF, 1, 0, 0, 1);
    matrix.render(config.getCompositionMode());

    delay(6000);

    startupFinished = true;
  } else if (resetButton.isPressed()) {
    char resetTimeString[16];
    itoa(resetButton.getPressDuration(), resetTimeString, 10);
    matrix.getTextLayer().clear();
    matrix.getTextLayer().setCursor(0, 0);
    matrix.getTextLayer().println("Reset");
    matrix.getTextLayer().setCursor(0, 16);
    matrix.getTextLayer().println(resetTimeString);
  } else if (showText == true) {
    textDisplay.renderText();
  }

  customData.update();

  ws.cleanupClients();

  matrix.render(config.getCompositionMode());

  if (millis() - lastHeapCheck > 300000) {
    lastHeapCheck = millis();
    checkHeapAndLog();
  }

  wifiHandler.checkConnection();

  delay(100);
}