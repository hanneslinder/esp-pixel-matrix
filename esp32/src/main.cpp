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

#include "config/pins.h"
#include "config/secrets.h"
#include "config/settings.h"
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

// Configuration variable definitions (declared in settings.h)
const char* ntpServer = "at.pool.ntp.org";
const char* portalIP = "10.0.1.1";
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* locale = "en_US.UTF-8";
const char* hostname = "pixelclock";

// Moved OTA progress state into OTAUpdate module

MatrixController matrix;

bool startupFinished = false;

// clock options
boolean showText = true;
boolean lastshowText = false;
boolean forceUpdateTime = false;
uint16_t timeColor = 0xFFFF;
uint16_t dateColor = 0xFFFF;

int compositionMode = 0;
int brightness = DEFAULT_BRIGHTNESS;

// Mutable copies for runtime configuration
char currentTimezone[64];

TextItem textContent[5]
    = { { "%H:%M", 0xFFFF, 1, -3, 1, 2, 1 }, { "%d.%b", 0xFFFF, 3, -1, 1, 1, 2 } };

char* socketData;
int currSocketBufferIndex = 0;

// custom data
int customDataUpdateInterval = -1;
boolean customDataEnabled = false;
char customDataServer[128] = "";
unsigned long lastCustomDataUpdate = 0;

static unsigned long lastHeapCheck = 0;
static size_t minFreeHeap = SIZE_MAX;

// Initialize handlers and server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiConnectionHandler wifiHandler(server);
ResetButtonHandler resetButton(RESET_PIN, RESET_SHORT_PRESS_TIME);
TextDisplayHandler textDisplay(matrix, textContent, 5);
WebServerHandler webServer(server, ws);

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

  // Configure WebSocket for better stability
  ws.enable(true);

  // Initialize WebSocket handler with dependencies
  WebSocketHandler::init(&matrix, textContent, &ws, socketData, &currSocketBufferIndex,
      SOCKET_DATA_SIZE, &textDisplay);

  Serial.println("WebSocket initialized with safety limits");
}

String httpGETRequest(const char* serverName)
{
  HTTPClient http;

  http.begin(serverName);
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}

// Custom data allows the esp32 to poll data from remote server.
// This should work but is currently not implemented in the browser UI
void handleCustomData()
{
  // UpdateInterval is in seconds
  if (millis() - lastCustomDataUpdate > customDataUpdateInterval * 1000) {
    lastCustomDataUpdate = millis();

    char interval[16];
    itoa(customDataUpdateInterval, interval, 10);

    String data = httpGETRequest(customDataServer);

    Serial.println("New data :)");
    Serial.println(data);

    // textLayer.clear();
    // textLayer.drawText(customDataServer, MIDDLE, NULL, timeColor, 2, -4);
    // textLayer.drawText(interval, BOTTOM, NULL, dateColor, 1, -2);
  }
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

  // Initialize text display and set locale
  textDisplay.setLocale(locale);

  // char host[16];
  // snprintf(host, 16, "ESP%012llX", ESP.getEfuseMac());
  // MDNS.begin(host);
  // MDNS.addService("http", "tcp", 80);

  socketData = (char*)malloc(SOCKET_DATA_SIZE * sizeof(char));

  // Initialize reset button
  resetButton.begin();
  resetButton.onLongPress([]() { resetWifi(); });

  // Initialize mutable configuration with defaults
  strlcpy(currentTimezone, timezone, sizeof(currentTimezone));

  // Initialize WiFi connection
  wifiHandler.begin(useCaptivePortal, ssid, password);

  initMatrix();

  // Apply default brightness from settings
  matrix.setBrightness(brightness);
  Serial.printf("Set initial brightness to %d\n", brightness);

  initWebSocket();
  webServer.begin();

  // Set timezone
  if (currentTimezone != NULL) {
    Serial.printf("Setting timezone to %s\n", currentTimezone);
  } else {
    Serial.println("Timezone not set");
  }

  if (ntpServer != NULL) {
    Serial.printf("Setting NTP server to %s\n", ntpServer);
  } else {
    Serial.println("NTP server not set");
  }
  configTzTime(currentTimezone, ntpServer);

  // WiFi.setSleep(false);
}

void loop()
{
  wifiHandler.loop();

  resetButton.update();

  if (startupFinished == false) {
    const String ip = wifiHandler.getIPAddress();

    matrix.drawText(ip.c_str(), MIDDLE, &Picopixel, 0xFFFF, 1, 0, 0, 1);
    matrix.render(compositionMode);

    Serial.println("Startup finished, showing IP address for 6 seconds...");

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
  } else if (customDataUpdateInterval > -1 && strlen(customDataServer) > 0) {
    handleCustomData();
  }

  ws.cleanupClients();

  matrix.render(compositionMode);

  if (millis() - lastHeapCheck > 300000) {
    lastHeapCheck = millis();
    checkHeapAndLog();
  }

  // Check WiFi connection status
  wifiHandler.checkConnection();

  delay(100);
}