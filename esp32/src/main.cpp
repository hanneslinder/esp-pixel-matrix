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
#include "matrix/MatrixController.h"
#include "ota/OTAUpdateHandler.h"
#include "types/CommonTypes.h"
#include "utils/utils.h"
#include "websocket/WebSocketHandler.h"

#define U_PART U_SPIFFS

// Configuration variable definitions (declared in settings.h)
const char* ntpServer = "at.pool.ntp.org";
const char* portalIP = "10.0.1.1";
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* locale = "en_US.UTF-8";
const char* hostname = "pixelclock";

// Moved OTA progress state into OTAUpdate module

MatrixController matrix;

int lastResetButtonState = HIGH;
int currentResetButtonState = HIGH;
unsigned long resetButtonPressedTime = 0;
unsigned long resetButtonReleasedTime = 0;
unsigned long resetButtonPressDuration = 0;

bool startupFinished = false;

AsyncWebServer server(80);
Mycila::ESPConnect espConnect(server);
uint32_t lastLog = 0;

AsyncWebSocket ws("/ws");

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
char currentLocale[32];

static unsigned long lastWiFiCheck = 0;
static int wifiReconnectAttempts = 0;
static const int MAX_WIFI_RECONNECT_ATTEMPTS = 5;
static unsigned long wifiReconnectStartTime = 0;
static unsigned long lastHeapCheck = 0;
static size_t minFreeHeap = SIZE_MAX;

TextItem textContent[5]
    = { { "%H:%M", 0xFFFF, 1, -3, 1, 2, 1 }, { "%d.%b", 0xFFFF, 3, -1, 1, 1, 2 } };

char* socketData;
int currSocketBufferIndex = 0;

// custom data
int customDataUpdateInterval = -1;
boolean customDataEnabled = false;
char customDataServer[128] = "";
unsigned long lastCustomDataUpdate = 0;

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

void resetWifi()
{
  Serial.println("Reset WIFI settings!");

  // textLayer.clear();
  // textLayer.drawText("Reset", MIDDLE, NULL, timeColor, 6, -4);

  // wifiManager.resetSettings();
  delay(1000);

  ESP.restart();
  delay(1000);
}

// OTA UPDATE HANDLING START
// OTA-specific handlers moved to ota/OTAUpdateHandler.{h,cpp}

void setLocale()
{
  if (setlocale(LC_ALL, currentLocale) == NULL) {
    Serial.printf("Unable to set locale %s", currentLocale);
    return;
  }

  std::setlocale(LC_TIME, currentLocale);
  std::setlocale(LC_NUMERIC, currentLocale);

  Serial.printf("Set locale to %s", currentLocale);
}

// OTA UPDATE HANDLING END

void printTextItem(char text[], TextItem& t)
{
  textPosition pos = static_cast<textPosition>(t.line);

  if (t.font == 0) {
    matrix.drawText(text, pos, NULL, t.color, t.size, t.offsetX, t.offsetY, t.align);
  } else {
    matrix.drawText(text, pos, &Picopixel, t.color, t.size, t.offsetX, t.offsetY, t.align);
  }
}

void printText()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  // textLayer.clear();
  matrix.getTextLayer().clear();

  for (TextItem t : textContent) {
    char parsedDate[32];
    strftime(parsedDate, 32, t.text, &timeinfo);
    printTextItem(parsedDate, t);
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Configure WebSocket for better stability
  ws.enable(true);

  // Initialize WebSocket handler with dependencies
  WebSocketHandler::init(
      &matrix, textContent, &ws, socketData, &currSocketBufferIndex, SOCKET_DATA_SIZE);

  Serial.println("WebSocket initialized with safety limits");
}

String processHtmlTemplate(const String& var)
{
  if (var == "websocketUrl") {
    const String ip = WiFi.localIP().toString();
    return ip;
  }

  return String();
}

void initWebServer()
{
  server.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=14400");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", String(), false, processHtmlTemplate);
  });

  server.serveStatic("/main.css", SPIFFS, "/main.css").setCacheControl("max-age=14400");

  server.serveStatic("/main.js", SPIFFS, "/main.js").setCacheControl("max-age=14400");

  // OTA routes and progress callbacks are installed by the OTAUpdate module
  OTAUpdate::init(server, ws);

  server.begin();
}

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void initCaptivePortal()
{
  server.on("/clear", HTTP_GET, [&](AsyncWebServerRequest* request) {
    espConnect.clearConfiguration();
    request->send(200);
    ESP.restart();
  });

  // network state listener
  espConnect.listen(
      [](__unused Mycila::ESPConnect::State previous, __unused Mycila::ESPConnect::State state) {
        JsonDocument doc;
        espConnect.toJson(doc.to<JsonObject>());
        serializeJsonPretty(doc, Serial);
        Serial.println();
      });

  espConnect.setAutoRestart(true);
  espConnect.setBlocking(true);

  Serial.println("Trying to connect to saved WiFi or will start portal...");

  espConnect.begin("arduino", portalSsid, portalPassword);

  Serial.println("ESPConnect completed, continuing setup()...");
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

void checkResetButton()
{
  currentResetButtonState = digitalRead(RESET_PIN);

  if (currentResetButtonState == LOW && lastResetButtonState == LOW) {
    resetButtonPressDuration++;
  }

  if (lastResetButtonState == HIGH && currentResetButtonState == LOW) {
    resetButtonPressedTime = millis();
  } else if (lastResetButtonState == LOW && currentResetButtonState == HIGH) {
    resetButtonReleasedTime = millis();
    resetButtonPressDuration = 0;

    long pressDuration = resetButtonReleasedTime - resetButtonPressedTime;

    if (pressDuration > RESET_SHORT_PRESS_TIME) {
      Serial.println("RESET BUTTON PRESS");
      resetWifi();
    }
  }

  lastResetButtonState = currentResetButtonState;
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
      WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected",
      WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
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

  setLocale();

  // char host[16];
  // snprintf(host, 16, "ESP%012llX", ESP.getEfuseMac());
  // MDNS.begin(host);
  // MDNS.addService("http", "tcp", 80);

  pinMode(RESET_PIN, INPUT_PULLUP);
  socketData = (char*)malloc(SOCKET_DATA_SIZE * sizeof(char));

  // Initialize mutable configuration with defaults
  strlcpy(currentTimezone, timezone, sizeof(currentTimezone));
  strlcpy(currentLocale, locale, sizeof(currentLocale));

  if (useCaptivePortal == true) {
    initCaptivePortal();
  } else {
    connectToWiFi();
  }

  initMatrix();

  // Apply default brightness from settings
  matrix.setBrightness(brightness);
  Serial.printf("Set initial brightness to %d\n", brightness);

  initWebServer();
  initWebSocket();

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
  if (useCaptivePortal == true) {
    espConnect.loop();
  }

  checkResetButton();

  if (startupFinished == false) {
    const String ip = WiFi.localIP().toString();

    matrix.drawText(ip.c_str(), MIDDLE, &Picopixel, 0xFFFF, 1, 0, 0, 1);

    delay(6000);

    startupFinished = true;
  } else if (currentResetButtonState == LOW) {
    char resetTimeString[16];
    itoa(resetButtonPressDuration, resetTimeString, 10);
    matrix.getTextLayer().clear();
    matrix.getTextLayer().setCursor(0, 0);
    matrix.getTextLayer().println("Reset");
    matrix.getTextLayer().setCursor(0, 16);
    matrix.getTextLayer().println(resetTimeString);
  } else if (showText == true) {
    printText();
  } else if (customDataUpdateInterval > -1 && strlen(customDataServer) > 0) {
    handleCustomData();
  }

  ws.cleanupClients();

  matrix.render(compositionMode);

  // Periodic heap and system status monitoring (every 5 minutes)
  if (millis() - lastHeapCheck > 300000) {
    lastHeapCheck = millis();
    checkHeapAndLog();
  }

  // Check if WiFi is connected with robust reconnection logic
  if (millis() - lastWiFiCheck > 30000) {
    lastWiFiCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, attempting reconnection...");

      if (wifiReconnectAttempts == 0) {
        wifiReconnectStartTime = millis();
      }

      wifiReconnectAttempts++;
      Serial.printf(
          "Reconnection attempt %d/%d\n", wifiReconnectAttempts, MAX_WIFI_RECONNECT_ATTEMPTS);

      if (wifiReconnectAttempts >= MAX_WIFI_RECONNECT_ATTEMPTS) {
        // If multiple reconnect attempts failed, try full re-initialization
        Serial.println(
            "Multiple reconnect attempts failed, performing full WiFi re-initialization...");
        WiFi.disconnect(true);
        delay(1000);
        WiFi.mode(WIFI_STA);

        if (useCaptivePortal == true) {
          Serial.println("Restarting ESP to reinitialize captive portal...");
          ESP.restart();
        } else {
          WiFi.begin(ssid, password);
        }

        wifiReconnectAttempts = 0;
        wifiReconnectStartTime = 0;
      } else {
        // Try simple reconnect first
        WiFi.disconnect();
        delay(100);
        WiFi.reconnect();
      }
    } else {
      // WiFi is connected, reset reconnect counter
      if (wifiReconnectAttempts > 0) {
        Serial.printf("WiFi reconnected successfully after %d attempts (took %lu ms)\n",
            wifiReconnectAttempts, millis() - wifiReconnectStartTime);
        wifiReconnectAttempts = 0;
        wifiReconnectStartTime = 0;
      }
    }
  }

  delay(100);
}