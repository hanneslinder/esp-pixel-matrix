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
#include "utils/utils.h"

#define U_PART U_SPIFFS

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

struct TextItem {
  char text[32];
  uint16_t color;
  int8_t offsetX;
  int8_t offsetY;
  uint8_t align;
  uint8_t size;
  uint8_t line;
  uint8_t font;
};

TextItem textContent[5]
    = { { "%H:%M", 0xFFFF, 1, -3, 1, 2, 1 }, { "%d.%b", 0xFFFF, 3, -1, 1, 1, 2 } };

char* socketData;
int currSocketBufferIndex = 0;

// custom data
int customDataUpdateInterval = -1;
boolean customDataEnabled = false;
char customDataServer[128] = "";
unsigned long lastCustomDataUpdate = 0;

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);

void initMatrix() { matrix.begin(); }

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg,
    uint8_t* data, size_t len)
{
  switch (type) {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
        client->remoteIP().toString().c_str());
    // Reset buffer state on new connection
    currSocketBufferIndex = 0;
    if (socketData != NULL) {
      socketData[0] = '\0';
    }
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    // Clean up buffer state on disconnect
    currSocketBufferIndex = 0;
    if (socketData != NULL) {
      socketData[0] = '\0';
    }
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
    break;
  case WS_EVT_ERROR:
    Serial.printf("WebSocket error for client #%u\n", client->id());
    // Reset buffer on error
    currSocketBufferIndex = 0;
    if (socketData != NULL) {
      socketData[0] = '\0';
    }
    break;
  }
}

void handleDrawPixelMessage(JsonArray& data)
{
  for (JsonVariant d : data) {
    const uint16_t x = d["p"][0];
    const uint16_t y = d["p"][1];
    const char* color = d["c"];
    const uint16_t c = strtol(color, NULL, 16);
    matrix.getBackgroundLayer().drawPixel(x, y, c);
  }
}

void handleDrawImageMessage(JsonArray data)
{
  int row = 0;
  int index = 0;
  int x = 0;
  int y = 0;

  for (JsonVariant d : data) {
    x = index % 64;
    y = row;

    const uint16_t c = strtol(d, NULL, 16);
    matrix.getBackgroundLayer().drawPixel(x, y, c);

    // matrix.getBackgroundLayer().drawPixel(x, y, hexToCRGB(d));

    if (x == 63) {
      row++;
    }

    index++;
  }
}

void setText(JsonArray& text)
{
  int index = 0;

  memset(textContent, 0, sizeof(textContent));

  for (JsonVariant t : text) {
    textContent[index].offsetX = t["offsetX"].as<signed short>();
    textContent[index].offsetY = t["offsetY"].as<signed short>();
    textContent[index].size = t["size"].as<signed short>();
    textContent[index].align = t["align"].as<signed short>();
    textContent[index].line = t["line"].as<signed short>();
    textContent[index].font = t["font"].as<signed short>();
    strlcpy(textContent[index].text, t["text"], sizeof textContent[index].text);

    const uint16_t c = strtol(t["color"], NULL, 16);
    textContent[index].color = c;

    ++index;
  }
}

// Sending all pixels at once is not possible because of a lack of memory
// Sending the data line by line causes the message queue to fill up
// Therefore chunk response with a handful of lines per message
void sendPixels()
{
  layerPixels* pixels = matrix.getBackgroundLayer().pixels;
  int linesPerMessage = 4;

  for (int y = 0; y < 32 / linesPerMessage; y++) {
    JsonDocument doc;
    JsonArray data = doc["data"].to<JsonArray>();
    doc["action"] = "matrixPixels";
    doc["layer"] = "bg";
    doc["line-start"] = y * linesPerMessage;
    doc["line-end"] = y * linesPerMessage + linesPerMessage;

    for (int line = 0; line < linesPerMessage; line++) {
      int currentLine = y * linesPerMessage + line;
      JsonArray lineData = data.add<JsonArray>();

      for (int x = 0; x < 64; x++) {
        CRGB pixel = pixels->data[currentLine][x];

        if (currentLine == 0 && x == 63) {
          Serial.printf("Pixel RGB %d %d %d", pixel.r, pixel.g, pixel.b);
        }

        lineData.add(convertRgbToHex(pixel.r, pixel.g, pixel.b, true));
      }
    }

    String json;
    serializeJson(doc, json);
    ws.textAll(json);
  }
}

void sendState()
{
  JsonDocument doc;
  JsonArray textArray = doc["text"].to<JsonArray>();

  doc["action"] = "matrixSettings";
  doc["customData"] = customDataEnabled;
  doc["customDataServer"] = customDataServer;
  doc["customDataInterval"] = customDataUpdateInterval;
  doc["compositionMode"] = compositionMode;
  doc["brightness"] = brightness;
  doc["timezone"] = currentTimezone;
  doc["locale"] = currentLocale;

  for (TextItem t : textContent) {
    if (strcmp(t.text, "") != 0) {
      JsonObject textObject = textArray.add<JsonObject>();
      textObject["text"] = t.text;
      textObject["line"] = t.line;
      textObject["offsetX"] = t.offsetX;
      textObject["offsetY"] = t.offsetY;
      textObject["size"] = t.size;
      textObject["align"] = t.align;
      textObject["font"] = t.font;
      textObject["color"] = convert16BitTo32BitHexColor(t.color);
    }
  }

  String json;
  serializeJson(doc, json);
  ws.textAll(json);
}

void handleCustomData(JsonObject customData)
{
  if (customData["updateInterval"] != 0 && customData["updateInterval"] >= -1) {
    customDataEnabled = true;
    customDataUpdateInterval = customData["updateInterval"];

    if (customData["server"] != 0) {
      String s = customData["server"];
      strlcpy(customDataServer, s.c_str(), sizeof(customDataServer));

      Serial.println("Enabled custom data");
      Serial.println(customDataServer);
    }
  } else {
    customDataEnabled = false;
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

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len)
{
  AwsFrameInfo* info = (AwsFrameInfo*)arg;

  // Validate frame info to detect corruption
  if (info->len > SOCKET_DATA_SIZE || info->len == 0) {
    Serial.printf("ERROR: Invalid frame length detected: %u bytes (max: %d). Likely corrupted "
                  "frame, resetting buffer.\n",
        info->len, SOCKET_DATA_SIZE);
    currSocketBufferIndex = 0;
    if (socketData != NULL) {
      socketData[0] = '\0';
    }
    return;
  }

  // Data fit into one packet
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    Serial.println("Single packet data");

    const size_t capacity = 34000;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    const char* action = doc["action"];
    Serial.println("Websocket message received:");
    Serial.println(action);

    if (isStringEqual(action, "drawpixel")) {
      JsonArray data = doc["data"].as<JsonArray>();
      handleDrawPixelMessage(data);
    } else if (isStringEqual(action, "drawImage")) {
      JsonArray data = doc["data"].as<JsonArray>();
      handleDrawImageMessage(data);
    } else if (isStringEqual(action, "clear")) {
      matrix.getBackgroundLayer().clear();
      matrix.getTextLayer().clear();
    } else if (isStringEqual(action, "toggleClock")) {
      matrix.getTextLayer().clear();
      lastshowText = showText;
      showText = doc["visible"];
    } else if (isStringEqual(action, "fill")) {
      const char* color = doc["color"];
      const uint16_t c = strtol(color, NULL, 16);
      matrix.getBackgroundLayer().fillScreen(c);
    } else if (isStringEqual(action, "compositionMode")) {
      compositionMode = doc["mode"];
    } else if (isStringEqual(action, "getPixels")) {
      sendPixels();
    } else if (isStringEqual(action, "customData")) {
      JsonObject customData = doc["options"].as<JsonObject>();
      handleCustomData(customData);
    } else if (isStringEqual(action, "getState")) {
      sendState();
    } else if (isStringEqual(action, "reset")) {
      resetWifi();
    } else if (isStringEqual(action, "setBrightness")) {
      brightness = doc["brightness"].as<int>();
      int clampedBrightness = max(MIN_BRIGHTNESS, min(brightness, MAX_BRIGHTNESS));

      if (brightness < MIN_BRIGHTNESS) {
        Serial.printf("Warning: Brightness %d is below minimum %d, setting to %d\n", brightness,
            MIN_BRIGHTNESS, clampedBrightness);
      }

      Serial.printf("setBrightness: requested=%d, actual=%d (range: %d-%d)\n", brightness,
          clampedBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
      matrix.setBrightness(clampedBrightness);
    } else if (isStringEqual(action, "setText")) {
      JsonArray text = doc["text"].as<JsonArray>();
      setText(text);
    } else if (isStringEqual(action, "setTimeZone")) {
      const char* tz = doc["timezone"];
      strlcpy(currentTimezone, tz, sizeof(currentTimezone));
      configTzTime(currentTimezone, ntpServer);
      Serial.printf("Timezone updated to: %s\n", currentTimezone);
    } else if (isStringEqual(action, "setLocale")) {
      const char* loc = doc["locale"];
      strlcpy(currentLocale, loc, sizeof(currentLocale));
      setLocale();
    }
  } else {
    // Multi packet message,
    // wait until whole message is transmitted before attempt to draw image

    // First chunk - validate and reset if needed
    if (info->index == 0) {
      Serial.printf("Starting new multi-packet message: total expected %u bytes\n", info->len);

      // Validate expected total length
      if (info->len > SOCKET_DATA_SIZE) {
        Serial.printf(
            "ERROR: Expected message size %u exceeds buffer size %d. Rejecting message.\n",
            info->len, SOCKET_DATA_SIZE);
        currSocketBufferIndex = 0;
        return;
      }

      // Reset buffer for new message
      currSocketBufferIndex = 0;
      if (socketData != NULL) {
        socketData[0] = '\0';
      }
    }

    // Add bounds checking to prevent buffer overflow
    if (currSocketBufferIndex + len > SOCKET_DATA_SIZE) {
      Serial.printf("ERROR: WebSocket buffer overflow detected! Curr: %d + New: %d > Max: %d. "
                    "Resetting buffer.\n",
          currSocketBufferIndex, len, SOCKET_DATA_SIZE);
      currSocketBufferIndex = 0;
      socketData[0] = '\0';
      return;
    }

    // Copy data to buffer
    memcpy(socketData + currSocketBufferIndex, data, len);
    currSocketBufferIndex += len;

    Serial.printf("Multi packet data: received %d/%u bytes (%.1f%%)\n", currSocketBufferIndex,
        info->len, (currSocketBufferIndex * 100.0) / info->len);

    // Check if we've received the complete message
    if (info->final && currSocketBufferIndex >= info->len) {
      Serial.printf("Complete message received: %d bytes\n", currSocketBufferIndex);

      // Null-terminate the buffer for safety
      if (currSocketBufferIndex < SOCKET_DATA_SIZE) {
        socketData[currSocketBufferIndex] = '\0';
      }

      // Use DynamicJsonDocument with calculated size for better memory management
      const size_t capacity = JSON_ARRAY_SIZE(2048) + JSON_OBJECT_SIZE(10) + currSocketBufferIndex;
      DynamicJsonDocument doc(capacity);

      DeserializationError error = deserializeJson(doc, socketData, currSocketBufferIndex);

      if (error) {
        Serial.printf("deserializeJson for large message failed: %s (size: %d bytes)\n",
            error.c_str(), currSocketBufferIndex);
        Serial.printf("Free heap before parse: %u bytes\n", ESP.getFreeHeap());

        // Reset buffer on error
        currSocketBufferIndex = 0;
        socketData[0] = '\0';
        return;
      }

      const char* action = doc["action"];
      Serial.printf("Processing action: %s\n", action);

      if (isStringEqual(action, "drawImage")) {
        JsonArray data = doc["data"].as<JsonArray>();
        handleDrawImageMessage(data);
      }

      // Always reset buffer after processing
      currSocketBufferIndex = 0;
      socketData[0] = '\0';

      Serial.println("Multi-packet message processed successfully");
    } else if (!info->final) {
      // More packets expected
      Serial.println("Waiting for more packets...");
    }
  }
}

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