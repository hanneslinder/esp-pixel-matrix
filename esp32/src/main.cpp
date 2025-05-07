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
#include "utils/utils.h"

#define U_PART U_SPIFFS

size_t updateContentLength;
int lastUpdateProgress = 0;

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
int brightness = 2;

static unsigned long lastWiFiCheck = 0;

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
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
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
  doc["timezone"] = timezone;
  doc["locale"] = locale;

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
      strcpy(customDataServer, s.c_str());

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
void handleUpdate(AsyncWebServerRequest* request)
{
  char* html = "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input "
               "type='file' name='update'><input type='submit' value='Update'></form>";
  request->send(200, "text/html", html);
}

void handleDoUpdate(AsyncWebServerRequest* request, const String& filename, size_t index,
    uint8_t* data, size_t len, bool final)
{
  if (!index) {
    Serial.println("Update");
    updateContentLength = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;

    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  if (final) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "updatefinished");
    response->addHeader("Refresh", "20");
    response->addHeader("Location", "/");
    request->send(response);

    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      JsonDocument doc;
      doc["action"] = "updateProgress";
      doc["progress"] = 100;

      String json;
      serializeJson(doc, json);
      ws.textAll(json);

      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

void printUpdateProgress(size_t prg, size_t sz)
{
  int progress = (prg * 100) / updateContentLength;
  Serial.printf("Progress: %d%%\n", progress);

  if (progress == 0 || progress - lastUpdateProgress >= 5 || progress >= 99) {
    JsonDocument doc;
    doc["action"] = "updateProgress";
    doc["progress"] = progress;

    String json;
    serializeJson(doc, json);
    ws.textAll(json);

    lastUpdateProgress = progress;
  }
}

void setLocale()
{
  if (setlocale(LC_ALL, locale) == NULL) {
    Serial.printf("Unable to set locale %s", locale);
    return;
  }

  std::setlocale(LC_TIME, locale);
  std::setlocale(LC_NUMERIC, locale);

  Serial.printf("Set locale to %s", locale);
}

// OTA UPDATE HANDLING END

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len)
{
  AwsFrameInfo* info = (AwsFrameInfo*)arg;

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
      // sendPixels();
    } else if (isStringEqual(action, "customData")) {
      JsonObject customData = doc["options"].as<JsonObject>();
      handleCustomData(customData);
    } else if (isStringEqual(action, "getState")) {
      sendState();
    } else if (isStringEqual(action, "reset")) {
      resetWifi();
    } else if (isStringEqual(action, "setBrightness")) {
      brightness = doc["brightness"].as<int>();
      // matrix->setPanelBrightness(min(brightness, MAX_BRIGHTNESS));
      matrix.setBrightness(min(brightness, MAX_BRIGHTNESS));
    } else if (isStringEqual(action, "setText")) {
      JsonArray text = doc["text"].as<JsonArray>();
      setText(text);
    } else if (isStringEqual(action, "setTimeZone")) {
      timezone = doc["timezone"];
    } else if (isStringEqual(action, "setLocale")) {
      locale = doc["locale"];
      setLocale();
    }
  } else {
    // Multi packet message,
    // wait until whole message is transmitted before attempt to draw image
    for (size_t i = 0; i < len; i++) {
      socketData[currSocketBufferIndex] = data[i];
      currSocketBufferIndex++;
    }

    Serial.println("Multi packet data");

    if (currSocketBufferIndex >= info->len) {
      const size_t capacity = 48000;
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, socketData);

      if (error) {
        Serial.print(F("deserializeJson for large message failed: "));
        Serial.println(error.f_str());
        return;
      }

      const char* action = doc["action"];

      if (isStringEqual(action, "drawImage")) {
        JsonArray data = doc["data"].as<JsonArray>();

        handleDrawImageMessage(data);
        socketData[0] = 0;
        currSocketBufferIndex = 0;
      }
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

  // Firmware update route
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) { handleUpdate(request); });

  server.on(
      "/doUpdate", HTTP_POST, [](AsyncWebServerRequest* request) {},
      [](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data,
          size_t len, bool final) { handleDoUpdate(request, filename, index, data, len, final); });

  Update.onProgress(printUpdateProgress);

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

  initMatrix();

  if (useCaptivePortal == true) {
    initCaptivePortal();
  } else {
    connectToWiFi();
  }

  initWebServer();
  initWebSocket();

  // Set timezone
  if (timezone != NULL) {
    Serial.printf("Setting timezone to %s\n", timezone);
  } else {
    Serial.println("Timezone not set");
  }

  if (ntpServer != NULL) {
    Serial.printf("Setting NTP server to %s\n", ntpServer);
  } else {
    Serial.println("NTP server not set");
  }
  configTzTime(timezone, ntpServer);

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

    // Cannot use text layer here as we have a `delay` that blocks the rendering

    matrix.getTextLayer().clear();
    matrix.getTextLayer().setCursor(9, 13);
    matrix.getTextLayer().setFont(&Picopixel);
    matrix.getTextLayer().println(ip);

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
  } else if (customDataUpdateInterval > -1 && customDataServer != "") {
    handleCustomData();
  }

  ws.cleanupClients();

  matrix.render(compositionMode);

  // // Check if WiFi is connected
  if (millis() - lastWiFiCheck > 30000) {
    lastWiFiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  delay(100);
}