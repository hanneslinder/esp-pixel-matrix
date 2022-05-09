#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Wire.h>

#include "Layer.h"
#include "SPIFFS.h"
#include "time.h"
#include "utils.h"

#include <Update.h>
#include <ESPmDNS.h>
#define U_PART U_SPIFFS
size_t content_len;
int lastUpdateProgress = 0;

#define USE_CUSTOM_PINS
#define A_PIN 5
#define B_PIN 19
#define C_PIN 22
#define D_PIN 21
#define E_PIN 12

#define R1_PIN 2
#define R2_PIN 18
#define G1_PIN 15
#define G2_PIN 27
#define B1_PIN 4
#define B2_PIN 14

#define CLK_PIN 23
#define LAT_PIN 25
#define OE_PIN 26

#define RESET_PIN 32

const int RESET_SHORT_PRESS_TIME = 2000;
int lastResetButtonState = HIGH;
int currentResetButtonState = HIGH;
unsigned long resetButtonPressedTime  = 0;
unsigned long resetButtonReleasedTime = 0;
unsigned long resetButtonPressDuration = 0;

MatrixPanel_I2S_DMA matrix;
Layer bgLayer(matrix);
Layer textLayer(matrix);

// WIFI Portal config
const char *ntpServer = "at.pool.ntp.org";
const char *portalSsid = "PixelClock";
const char *portalPassword = "clockpixel";
const char *portalIP = "10.0.1.1";

// Replace with your timezone
// https://ftp.fau.de/aminet/util/time/tzinfo.txt
const char *timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

// locale does not work at the moment, ESP-IDF does not support it
const char *locale = "en_US.UTF-8";

// Flag to display IP address on startup
bool startupFinished = false;

WiFiManager wifiManager;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int compositionMode = 0;
int brightness = 2;
const int MAX_BRIGHTNESS = 15;

// clock options
boolean showText = true;
boolean lastshowText = false;
boolean forceUpdateTime = false;
uint16_t timeColor = 0xFFFF;
uint16_t dateColor = 0xFFFF;

struct TextItem {
  char text[32];
  uint16_t color;
  int8_t offsetX;
  int8_t offsetY;
  uint8_t align;
  uint8_t size;
  uint8_t line;
};

TextItem textContent[5] = {
  {"%H:%M", 0xFFFF, 1, -3, 1, 2, 1},
  {"%d.%b", 0xFFFF, 3, -1, 1, 1, 2}
};

const size_t SOCKET_DATA_SIZE = 48000;
char * socketData;
int currSocketBufferIndex = 0;

// custom data
int customDataUpdateInterval = -1;
boolean customDataEnabled = false;
char customDataServer[128] = "";
unsigned long lastCustomDataUpdate = 0;

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

void initMatrix() {
  matrix.begin(R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN);

  bgLayer.init();
  bgLayer.clear();
  bgLayer.setTransparency(false);

  textLayer.init();
  textLayer.clear();

  matrix.setPanelBrightness(2);
  matrix.clearScreen();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
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

void handleDrawPixelMessage(JsonArray &data) {
  for (JsonVariant d : data) {
    const uint16_t x = d["p"][0];
    const uint16_t y = d["p"][1];
    const char *color = d["c"];
    const uint16_t c = strtol(color, NULL, 16);
    bgLayer.drawPixel(x, y, c);
  }
}

void handleDrawImageMessage(JsonArray data) {
  int row = 0;
  int index = 0;
  int x = 0;
  int y = 0;

  for (JsonVariant d : data) {
    x = index % 64;
    y = row;

    const uint16_t c = strtol(d, NULL, 16);
    bgLayer.drawPixel(x, y, c);

    if (x == 63) {
      row++;
    }

    index++;
  }
}

void setText(JsonArray &text) {
  int index = 0;

  memset(textContent, 0, sizeof(textContent));

  for (JsonVariant t : text) {
    textContent[index].offsetX = t["offsetX"].as<signed short>();
    textContent[index].offsetY = t["offsetY"].as<signed short>();
    textContent[index].size = t["size"].as<signed short>();
    textContent[index].align = t["align"].as<signed short>();
    textContent[index].line = t["line"].as<signed short>();
    strlcpy(textContent[index].text, t["text"], sizeof textContent[index].text);

    const uint16_t c = strtol(t["color"], NULL, 16);
    textContent[index].color = c;

    ++index;
  }
}

std::string rgb2hex(int r, int g, int b, bool with_head = false);

std::string rgb2hex(int r, int g, int b, bool with_head) {
  std::stringstream ss;
  ss << std::hex << (r << 16 | g << 8 | b);
  
  std::string hex = ss.str();
  int str_length = hex.length();

	for (int i = 0; i < 6 - str_length; i++) {
		hex = "0" + hex;
  }

  if (with_head) {
    hex = "#" + hex;
  }

	return hex;
}

// Sending all pixels at once is not possible because of a lack of memory 
// Sending the data line by line causes the message queue to fill up
// Therefore chunk response with a handful of lines per message
void sendPixels() {
 	layerPixels * pixels = bgLayer.getPixels();
  int linesPerMessage = 4;

	for (int y = 0; y < 32 / linesPerMessage; y++) {
		StaticJsonDocument<8192> doc;
		JsonArray data = doc.createNestedArray("data");
		doc["action"] = "matrixPixels";
		doc["layer"] = "bg";
    doc["line-start"] = y * linesPerMessage;
    doc["line-end"] = y * linesPerMessage + linesPerMessage;
		

    for (int line = 0; line < linesPerMessage; line++) {
      int currentLine = y * linesPerMessage + line;
      JsonArray lineData = data.createNestedArray();

      for (int x = 0; x < 64; x++) {
        CRGB pixel = pixels->data[currentLine][x];

        if (currentLine == 0 && x == 63) {
          Serial.printf("Pixel RGB %d %d %d", pixel.r, pixel.g, pixel.b);
        }

        lineData.add(rgb2hex(pixel.r, pixel.g, pixel.b, true));
      }
    }

		String json;
    serializeJson(doc, json);
		ws.textAll(json);
	}
}

// Convert 5-6-5 color to 32bit hex value
String convert16BitTo32BitHexColor(uint16_t hexValue) {
  unsigned r = (hexValue & 0xF800) >> 11;
  unsigned g = (hexValue & 0x07E0) >> 5;
  unsigned b = hexValue & 0x001F;

  r = (r * 255) / 31;
  g = (g * 255) / 63;
  b = (b * 255) / 31;

  char hex[8] = {0};
  sprintf(hex, "#%02X%02X%02X", r, g, b);
  return hex;
}

void sendState() {
	StaticJsonDocument<768> doc;
  JsonArray textArray = doc.createNestedArray("text");

  doc["action"] = "matrixSettings";
	doc["customData"] = customDataEnabled;
	doc["customDataServer"] = customDataServer;
	doc["customDataInterval"] = customDataUpdateInterval;
	doc["compositionMode"] = compositionMode;
  doc["brightness"] = brightness;
  doc["timezone"] = timezone;
  doc["locale"] = locale;

  for (TextItem t : textContent) {
    JsonObject textObject = textArray.createNestedObject();
    textObject["text"] = t.text;
    textObject["line"] = t.line;
    textObject["offsetX"] = t.offsetX;
    textObject["offsetY"] = t.offsetY;
    textObject["size"] = t.size;
    textObject["align"] = t.align;
    textObject["color"] = convert16BitTo32BitHexColor(t.color);
  }

	String json;
	serializeJson(doc, json);
	ws.textAll(json);
}

void handleCustomData(JsonObject customData) {
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

void resetWifi() {
  Serial.println("Reset WIFI settings!");

  textLayer.clear();
	textLayer.drawText("Reset", MIDDLE, NULL, timeColor, 6, -4);

  wifiManager.resetSettings();
  delay(1000);

  ESP.restart();
  delay(1000);
}

// OTA UPDATE HANDLING START
void handleUpdate(AsyncWebServerRequest *request) {
  char* html = "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  request->send(200, "text/html", html);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index){
    Serial.println("Update");
    content_len = request->contentLength();
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
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "update finished");
    response->addHeader("Refresh", "20");  
    response->addHeader("Location", "/");
    request->send(response);

    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      StaticJsonDocument<265> doc;
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

void printUpdateProgress(size_t prg, size_t sz) {
  int progress = (prg * 100) / content_len;
  Serial.printf("Progress: %d%%\n", progress);

  if (progress == 0 || progress - lastUpdateProgress >= 5 || progress >= 99) {
    StaticJsonDocument<265> doc;
    doc["action"] = "updateProgress";
    doc["progress"] = progress;

    String json;
    serializeJson(doc, json);
    ws.textAll(json);

    lastUpdateProgress = progress;
  } 
}

void setLocale() {
  if (setlocale(LC_ALL, locale) == NULL) {
    Serial.printf("Unable to set locale %s", locale);
    return;
  }

  std::setlocale(LC_TIME, locale);
  std::setlocale(LC_NUMERIC, locale);

  Serial.printf("Set locale to %s", locale);
}

// OTA UPDATE HANDLING END

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;

  // Data fit into one packet 
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

    Serial.println("Single packet data");
  
    const size_t capacity = 34000;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    const char *action = doc["action"];
    Serial.println("Websocket message received:");
    Serial.println(action);

    if (isStringEqual(action, "drawpixel")) {
      JsonArray data = doc["data"].as<JsonArray>();
      handleDrawPixelMessage(data);
    } else if (isStringEqual(action, "drawImage")) {
      JsonArray data = doc["data"].as<JsonArray>();
      handleDrawImageMessage(data);
    } else if (isStringEqual(action, "clear")) {
      bgLayer.clear();
			textLayer.clear();
    } else if (isStringEqual(action, "toggleClock")) {
      textLayer.clear();
      lastshowText = showText;
      showText = doc["visible"];
    } else if (isStringEqual(action, "fill")) {
      const char *color = doc["color"];
      const uint16_t c = strtol(color, NULL, 16);
      bgLayer.fillScreen(c);
    } else if (isStringEqual(action, "compositionMode")) {
      compositionMode = doc["mode"];
    } else if (isStringEqual(action, "getPixels")) {
      sendPixels();
    } else if (isStringEqual(action, "customData")) {
			JsonObject customData = doc["options"].as<JsonObject>();
			handleCustomData(customData);
		} else if(isStringEqual(action, "getState")) {
			sendState();
		} else if(isStringEqual(action, "reset")) {
			resetWifi();
		} else if(isStringEqual(action, "setBrightness")) {
      brightness = doc["brightness"].as<int>();
      matrix.setPanelBrightness(min(brightness, MAX_BRIGHTNESS));
    } else if(isStringEqual(action, "setText")) {
      JsonArray text = doc["text"].as<JsonArray>();
      setText(text);
    } else if(isStringEqual(action, "setTimeZone")) {
      timezone = doc["timezone"];
    } else if(isStringEqual(action, "setLocale")) {
      locale = doc["locale"];
      setLocale();
    }
  } else {
    // Multi packet message,
    // wait until whole message is transmitted before attempt to draw image
    for (size_t i = 0; i < len; i++){
      socketData[currSocketBufferIndex] = data[i];
      currSocketBufferIndex++;
    }

    Serial.println("Multi packet data");

    if(currSocketBufferIndex >= info->len) {
      const size_t capacity = 48000;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, socketData);

      if (error) {
        Serial.print(F("deserializeJson for large message failed: "));
        Serial.println(error.f_str());
        return;
      }

      const char *action = doc["action"];

      if (isStringEqual(action, "drawImage")) {
        JsonArray data = doc["data"].as<JsonArray>();

        handleDrawImageMessage(data);
        socketData[0] = 0;
        currSocketBufferIndex = 0;
      }
    }
  }
}

void printTextItem(char text[], TextItem &t) {
  textPosition pos = static_cast<textPosition>(t.line);
  textLayer.drawText(text, pos, NULL, t.color, t.size, t.offsetX, t.offsetY, t.align);
}

void printText() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  textLayer.clear();

  for (TextItem t : textContent) {
    char parsedDate[32];
    strftime(parsedDate, 32, t.text, &timeinfo);
    printTextItem(parsedDate, t);
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processHtmlTemplate(const String &var) {
  if (var == "websocketUrl") {
    const String ip = WiFi.localIP().toString();
		return ip;
  }

  return String();
}

void initWebServer() {
	server.serveStatic("/", SPIFFS, "/");

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    handleUpdate(request);
  });

  server.on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest *request) {}, [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
    handleDoUpdate(request, filename, index, data, len, final);
  });

  Update.onProgress(printUpdateProgress);

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/index.html", String(), false, processHtmlTemplate);
	});

	server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/main.css", "text/css");
	});

	server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/main.js", "application/javascript");
	});
}

void configModeCallback(WiFiManager *myWiFiManager) {
  matrix.clearScreen();
  matrix.setTextSize(1);
  matrix.setCursor(20, 2);
  matrix.print("WIFI:");
  matrix.setCursor(3, 12);
  matrix.print(portalSsid);
  matrix.setCursor(9, 22);
  matrix.print(portalIP);
}

void initCaptivePortal() {
  wifiManager.setAPCallback(configModeCallback);

  textLayer.clear();
  textLayer.drawText("Connect:", TOP, NULL, timeColor, 1, 0);
  textLayer.drawText(portalSsid, MIDDLE, NULL, timeColor, 1, 0);
  textLayer.drawText(portalIP, BOTTOM, NULL, timeColor, 1, 0);

  wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

  if (!wifiManager.autoConnect(portalSsid)) {
    ESP.restart();
    delay(1000);
  }
}

String httpGETRequest(const char* serverName) {
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
void handleCustomData() {
	// UpdateInterval is in seconds
	if (millis() - lastCustomDataUpdate > customDataUpdateInterval * 1000) {
		lastCustomDataUpdate = millis();

		char interval[16];
		itoa(customDataUpdateInterval, interval, 10);

		String data = httpGETRequest(customDataServer);

		Serial.println("New data :)");
		Serial.println(data);

		textLayer.clear();
		textLayer.drawText(customDataServer, MIDDLE, NULL, timeColor, 2, -4);
		textLayer.drawText(interval, BOTTOM, NULL, dateColor, 1, -2);
	}
}

void checkResetButton() {
  currentResetButtonState = digitalRead(RESET_PIN);

  if (currentResetButtonState == LOW && lastResetButtonState == LOW) {
    resetButtonPressDuration++;
  }

  if(lastResetButtonState == HIGH && currentResetButtonState == LOW) {
    resetButtonPressedTime = millis();

  } else if(lastResetButtonState == LOW && currentResetButtonState == HIGH) {
    resetButtonReleasedTime = millis();
    resetButtonPressDuration = 0;

    long pressDuration = resetButtonReleasedTime - resetButtonPressedTime;

    if(pressDuration > RESET_SHORT_PRESS_TIME) {
      Serial.println("RESET BUTTON PRESS");
      resetWifi();
    }
  }

  lastResetButtonState = currentResetButtonState;
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();

  setLocale();

  char host[16];
  snprintf(host, 16, "ESP%012llX", ESP.getEfuseMac());
  MDNS.begin(host);
  MDNS.addService("http", "tcp", 80);

  pinMode(RESET_PIN, INPUT_PULLUP);
  socketData = (char *) malloc (SOCKET_DATA_SIZE * sizeof (char));

  initMatrix();
  initCaptivePortal();
	initWebServer();
	initWebSocket();
  configTzTime(timezone, ntpServer);

  server.begin();
}

void loop() {
  checkResetButton();

  if (startupFinished == false) {
    const String ip = WiFi.localIP().toString();

    matrix.clearScreen();
    matrix.setTextSize(1);
    matrix.setCursor(3, 2);
    matrix.print(ip);

    delay(6000);

    startupFinished = true;
  } else if(currentResetButtonState == LOW) {
    char resetTimeString[16];
    itoa(resetButtonPressDuration, resetTimeString, 10);
    textLayer.clear();
		textLayer.drawText("Reset", MIDDLE, NULL, timeColor, 2, 1);
		textLayer.drawText(resetTimeString, BOTTOM, NULL, dateColor, 1, -2);
  } else if (showText == true) {
    printText();
  } else if(customDataUpdateInterval > -1 && customDataServer != "") {
		handleCustomData();
	}

  ws.cleanupClients();

  switch (compositionMode) {
    case 0:
      LayerCompositor::Stack(matrix, bgLayer, textLayer);
      break;
    case 1:
      LayerCompositor::Blend(matrix, bgLayer, textLayer);
      break;
    case 2:
      LayerCompositor::Siloette(matrix, bgLayer, textLayer);
      break;
		default:
			LayerCompositor::Stack(matrix, bgLayer, textLayer);
			break;
  }

  delay(100);
}