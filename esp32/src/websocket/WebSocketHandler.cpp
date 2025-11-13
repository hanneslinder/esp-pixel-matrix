#include "WebSocketHandler.h"
#include "../config/ConfigManager.h"
#include "../config/settings.h"
#include "../data/CustomDataHandler.h"
#include "../display/TextDisplayHandler.h"
#include "../matrix/MatrixController.h"
#include "../utils/utils.h"
#include "SPIFFS.h"
#include "time.h"
#include <Fonts/Picopixel.h>

// External dependencies
extern char currentTimezone[64];
extern boolean showText;
extern boolean lastshowText;

// Custom data externals (for backward compatibility with UI)
extern int customDataUpdateInterval;
extern boolean customDataEnabled;
extern char customDataServer[128];

// Forward declaration of external functions
extern void resetWifi();

namespace WebSocketHandler {

// Get ConfigManager instance
static ConfigManager& config = ConfigManager::getInstance();

static MatrixController* matrix = nullptr;
static TextItem* textContent = nullptr;
static AsyncWebSocket* ws = nullptr;
static char* socketData = nullptr;
static int* currSocketBufferIndex = nullptr;
static int socketBufferSize = 0;
static TextDisplayHandler* textDisplay = nullptr;
static CustomDataHandler* customData = nullptr;

// ============================================================================
// INITIALIZATION
// ============================================================================

void init(MatrixController* matrixCtrl, TextItem* textItems, AsyncWebSocket* websocket,
    char* socketBuffer, int* bufferIndex, const int bufferSize,
    TextDisplayHandler* textDisplayHandler, CustomDataHandler* customDataHandler)
{
  matrix = matrixCtrl;
  textContent = textItems;
  ws = websocket;
  socketData = socketBuffer;
  currSocketBufferIndex = bufferIndex;
  socketBufferSize = bufferSize;
  textDisplay = textDisplayHandler;
  customData = customDataHandler;

  Serial.println("WebSocketHandler initialized");
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void resetBuffer()
{
  *currSocketBufferIndex = 0;
  if (socketData != nullptr) {
    socketData[0] = '\0';
  }
}

// ============================================================================
// MESSAGE HANDLERS - Drawing Operations
// ============================================================================

void handleDrawPixel(JsonDocument& doc)
{
  JsonArray data = doc["data"].as<JsonArray>();

  for (JsonVariant d : data) {
    const uint16_t x = d["p"][0];
    const uint16_t y = d["p"][1];
    const char* color = d["c"];
    const uint16_t c = strtol(color, NULL, 16);
    matrix->getBackgroundLayer().drawPixel(x, y, c);
  }
}

void handleDrawImage(JsonDocument& doc)
{
  JsonArray data = doc["data"].as<JsonArray>();
  int row = 0;
  int index = 0;

  for (JsonVariant d : data) {
    int x = index % 64;
    int y = row;

    const uint16_t c = strtol(d, NULL, 16);
    matrix->getBackgroundLayer().drawPixel(x, y, c);

    if (x == 63) {
      row++;
    }
    index++;
  }
}

void handleClear(JsonDocument& doc)
{
  matrix->getBackgroundLayer().clear();
  matrix->getTextLayer().clear();
}

void handleFill(JsonDocument& doc)
{
  const char* color = doc["color"];
  const uint16_t c = strtol(color, NULL, 16);
  matrix->getBackgroundLayer().fillScreen(c);
}

// ============================================================================
// MESSAGE HANDLERS - Text and Clock Operations
// ============================================================================

void handleToggleClock(JsonDocument& doc)
{
  matrix->getTextLayer().clear();
  lastshowText = showText;
  showText = doc["visible"];
}

void handleSetText(JsonDocument& doc)
{
  JsonArray text = doc["text"].as<JsonArray>();
  int index = 0;

  memset(textContent, 0, sizeof(TextItem) * 5);

  for (JsonVariant t : text) {
    textContent[index].offsetX = t["offsetX"].as<signed short>();
    textContent[index].offsetY = t["offsetY"].as<signed short>();
    textContent[index].size = t["size"].as<signed short>();
    textContent[index].align = t["align"].as<signed short>();
    textContent[index].line = t["line"].as<signed short>();
    textContent[index].font = t["font"].as<signed short>();
    strlcpy(textContent[index].text, t["text"], sizeof(textContent[index].text));

    const uint16_t c = strtol(t["color"], NULL, 16);
    textContent[index].color = c;

    ++index;
  }
}

// ============================================================================
// MESSAGE HANDLERS - Configuration Operations
// ============================================================================

void handleCompositionMode(JsonDocument& doc)
{
  int mode = doc["mode"];
  config.setCompositionMode(mode);
  config.save();
  broadcastConfigUpdate();
}

void handleSetBrightness(JsonDocument& doc)
{
  int brightness = doc["brightness"].as<int>();
  int clampedBrightness = max(MIN_BRIGHTNESS, min(brightness, MAX_BRIGHTNESS));

  if (brightness < MIN_BRIGHTNESS) {
    Serial.printf("Warning: Brightness %d is below minimum %d, setting to %d\n", brightness,
        MIN_BRIGHTNESS, clampedBrightness);
  }

  Serial.printf("setBrightness: requested=%d, actual=%d (range: %d-%d)\n", brightness,
      clampedBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);

  config.setBrightness(clampedBrightness);
  matrix->setBrightness(clampedBrightness);
  config.save();
  broadcastConfigUpdate();
}

void handleSetTimeZone(JsonDocument& doc)
{
  const char* tz = doc["timezone"];
  config.setTimezone(tz);
  strlcpy(currentTimezone, tz, sizeof(currentTimezone)); // Keep for backward compatibility
  configTzTime(config.getTimezone(), config.getNtpServer());
  config.save();
  broadcastConfigUpdate();
  Serial.printf("Timezone updated to: %s\n", tz);
}

void handleSetLocale(JsonDocument& doc)
{
  const char* loc = doc["locale"];
  config.setLocale(loc);
  if (textDisplay != nullptr) {
    textDisplay->setLocale(loc);
  }
  config.save();
  broadcastConfigUpdate();
  Serial.printf("Locale updated to: %s\n", loc);
}

void handleCustomData(JsonDocument& doc)
{
  JsonObject customDataObj = doc["options"].as<JsonObject>();

  if (customDataObj["updateInterval"] != 0 && customDataObj["updateInterval"] >= -1) {
    int interval = customDataObj["updateInterval"];

    if (customDataObj["server"] != 0) {
      String serverUrl = customDataObj["server"];

      // Update ConfigManager
      config.setCustomDataEnabled(true);
      config.setCustomDataInterval(interval);
      config.setCustomDataServer(serverUrl.c_str());

      // Update CustomDataHandler
      if (customData != nullptr) {
        customData->setEnabled(true);
        customData->setUpdateInterval(interval);
        customData->setServerUrl(serverUrl.c_str());
      }

      // Keep extern variables for backward compatibility
      customDataEnabled = true;
      customDataUpdateInterval = interval;
      strlcpy(customDataServer, serverUrl.c_str(), sizeof(customDataServer));

      config.save();
      broadcastConfigUpdate();
      Serial.println("Enabled custom data");
      Serial.println(serverUrl);
    }
  } else {
    // Disable custom data
    config.setCustomDataEnabled(false);
    if (customData != nullptr) {
      customData->setEnabled(false);
    }
    customDataEnabled = false;
    config.save();
    broadcastConfigUpdate();
    Serial.println("Disabled custom data");
  }
}

// ============================================================================
// MESSAGE HANDLERS - Query Operations
// ============================================================================

void handleGetPixels(JsonDocument& doc) { sendPixels(); }

void handleGetState(JsonDocument& doc) { sendState(); }

void handleReset(JsonDocument& doc) { resetWifi(); }

// ============================================================================
// SEND FUNCTIONS
// ============================================================================

void sendPixels()
{
  layerPixels* pixels = matrix->getBackgroundLayer().pixels;
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
    ws->textAll(json);
  }
}

void sendState()
{
  JsonDocument doc;
  JsonArray textArray = doc["text"].to<JsonArray>();

  doc["action"] = "matrixSettings";

  // Use ConfigManager for all configuration values
  doc["customData"] = config.isCustomDataEnabled();
  doc["customDataServer"] = config.getCustomDataServer();
  doc["customDataInterval"] = config.getCustomDataInterval();
  doc["compositionMode"] = config.getCompositionMode();
  doc["brightness"] = config.getBrightness();
  doc["timezone"] = config.getTimezone();
  doc["locale"] = config.getLocale();

  for (int i = 0; i < 5; i++) {
    if (strcmp(textContent[i].text, "") != 0) {
      JsonObject textObject = textArray.add<JsonObject>();
      textObject["text"] = textContent[i].text;
      textObject["line"] = textContent[i].line;
      textObject["offsetX"] = textContent[i].offsetX;
      textObject["offsetY"] = textContent[i].offsetY;
      textObject["size"] = textContent[i].size;
      textObject["align"] = textContent[i].align;
      textObject["font"] = textContent[i].font;
      textObject["color"] = convert16BitTo32BitHexColor(textContent[i].color);
    }
  }

  String json;
  serializeJson(doc, json);
  ws->textAll(json);
}

void broadcastConfigUpdate()
{
  if (ws != nullptr) {
    sendState();
    Serial.println("Config update broadcasted to all WebSocket clients");
  }
}

// ============================================================================
// ACTION DISPATCHER
// ============================================================================

void dispatchAction(const char* action, JsonDocument& doc)
{
  Serial.printf("Processing action: %s\n", action);

  // Drawing operations
  if (isStringEqual(action, "drawpixel")) {
    handleDrawPixel(doc);
  } else if (isStringEqual(action, "drawImage")) {
    handleDrawImage(doc);
  } else if (isStringEqual(action, "clear")) {
    handleClear(doc);
  } else if (isStringEqual(action, "fill")) {
    handleFill(doc);
  }
  // Text and clock operations
  else if (isStringEqual(action, "toggleClock")) {
    handleToggleClock(doc);
  } else if (isStringEqual(action, "setText")) {
    handleSetText(doc);
  }
  // Configuration operations
  else if (isStringEqual(action, "compositionMode")) {
    handleCompositionMode(doc);
  } else if (isStringEqual(action, "setBrightness")) {
    handleSetBrightness(doc);
  } else if (isStringEqual(action, "setTimeZone")) {
    handleSetTimeZone(doc);
  } else if (isStringEqual(action, "setLocale")) {
    handleSetLocale(doc);
  } else if (isStringEqual(action, "customData")) {
    handleCustomData(doc);
  }
  // Query operations
  else if (isStringEqual(action, "getPixels")) {
    handleGetPixels(doc);
  } else if (isStringEqual(action, "getState")) {
    handleGetState(doc);
  } else if (isStringEqual(action, "reset")) {
    handleReset(doc);
  } else {
    Serial.printf("Unknown action: %s\n", action);
  }
}

// ============================================================================
// SINGLE PACKET HANDLER
// ============================================================================

bool handleSinglePacket(uint8_t* data, size_t len)
{
  Serial.println("Single packet data");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data, len);

  if (error) {
    Serial.printf("deserializeJson() failed: %s\n", error.c_str());
    return false;
  }

  const char* action = doc["action"];
  Serial.printf("Websocket message received: %s\n", action);

  dispatchAction(action, doc);
  return true;
}

// ============================================================================
// MULTI-PACKET HANDLER
// ============================================================================

bool handleMultiPacket(AwsFrameInfo* info, uint8_t* data, size_t len)
{
  // First chunk - validate and reset if needed
  if (info->index == 0) {
    Serial.printf("Starting new multi-packet message: total expected %u bytes\n", info->len);

    // Validate expected total length
    if (info->len > socketBufferSize) {
      Serial.printf("ERROR: Expected message size %u exceeds buffer size %d. Rejecting message.\n",
          info->len, socketBufferSize);
      resetBuffer();
      return false;
    }

    // Reset buffer for new message
    resetBuffer();
  }

  // Bounds checking to prevent buffer overflow
  if (*currSocketBufferIndex + len > socketBufferSize) {
    Serial.printf("ERROR: WebSocket buffer overflow! Curr: %d + New: %d > Max: %d\n",
        *currSocketBufferIndex, len, socketBufferSize);
    resetBuffer();
    return false;
  }

  // Copy data to buffer
  memcpy(socketData + *currSocketBufferIndex, data, len);
  *currSocketBufferIndex += len;

  Serial.printf("Multi packet data: received %d/%u bytes (%.1f%%)\n", *currSocketBufferIndex,
      info->len, (*currSocketBufferIndex * 100.0) / info->len);

  // Check if we've received the complete message
  if (info->final && *currSocketBufferIndex >= info->len) {
    Serial.printf("Complete message received: %d bytes\n", *currSocketBufferIndex);

    // Null-terminate the buffer for safety
    if (*currSocketBufferIndex < socketBufferSize) {
      socketData[*currSocketBufferIndex] = '\0';
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, socketData, *currSocketBufferIndex);
    if (error) {
      Serial.printf("deserializeJson for large message failed: %s (size: %d bytes)\n",
          error.c_str(), *currSocketBufferIndex);
      Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
      resetBuffer();
      return false;
    }

    const char* action = doc["action"];
    dispatchAction(action, doc);

    // Reset buffer after processing
    resetBuffer();
    Serial.println("Multi-packet message processed successfully");
    return true;
  } else if (!info->final) {
    // More packets expected
    Serial.println("Waiting for more packets...");
  }

  return true;
}

// ============================================================================
// MAIN MESSAGE HANDLER
// ============================================================================

void handleMessage(void* arg, uint8_t* data, size_t len)
{
  AwsFrameInfo* info = (AwsFrameInfo*)arg;

  // Validate frame info to detect corruption
  if (info->len > socketBufferSize || info->len == 0) {
    Serial.printf("ERROR: Invalid frame length: %u bytes (max: %d). Corrupted frame detected.\n",
        info->len, socketBufferSize);
    resetBuffer();
    return;
  }

  // Single packet message
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    handleSinglePacket(data, len);
  }
  // Multi-packet message
  else {
    handleMultiPacket(info, data, len);
  }
} // end handleMessage

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void onConnect(AsyncWebSocketClient* client)
{
  Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
      client->remoteIP().toString().c_str());
  resetBuffer();
}

void onDisconnect(AsyncWebSocketClient* client)
{
  Serial.printf("WebSocket client #%u disconnected\n", client->id());
  resetBuffer();
}

void onError(AsyncWebSocketClient* client)
{
  Serial.printf("WebSocket error for client #%u\n", client->id());
  resetBuffer();
}

} // namespace WebSocketHandler
