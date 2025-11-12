#include "OTAUpdateHandler.h"

#include <Update.h>

// Using SPIFFS partition constant if defined in main; fallback to U_SPIFFS
#ifndef U_PART
#define U_PART U_SPIFFS
#endif

namespace OTAUpdate {

static AsyncWebSocket* g_ws = nullptr;
static size_t g_updateContentLength = 0;
static int g_lastUpdateProgress = 0;

static void sendProgress(int progress)
{
  if (!g_ws)
    return;

  JsonDocument doc;
  doc["action"] = "updateProgress";
  doc["progress"] = progress;

  String json;
  serializeJson(doc, json);
  g_ws->textAll(json);
}

static void handleUpdate(AsyncWebServerRequest* request)
{
  const char* html
      = "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  request->send(200, "text/html", html);
}

static void handleDoUpdate(
    AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final)
{
  if (!index) {
    Serial.println("Update");
    g_updateContentLength = request->contentLength();
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
    AsyncWebServerResponse* response
        = request->beginResponse(200, "text/plain", "updatefinished");
    response->addHeader("Refresh", "20");
    response->addHeader("Location", "/");
    request->send(response);

    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      sendProgress(100);
      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

static void printUpdateProgress(size_t prg, size_t sz)
{
  // Prefer the request content length if available, else use sz
  size_t total = g_updateContentLength > 0 ? g_updateContentLength : sz;
  int progress = total ? (int)((prg * 100) / total) : 0;
  Serial.printf("Progress: %d%%\n", progress);

  if (progress == 0 || progress - g_lastUpdateProgress >= 5 || progress >= 99) {
    sendProgress(progress);
    g_lastUpdateProgress = progress;
  }
}

void init(AsyncWebServer& server, AsyncWebSocket& ws)
{
  g_ws = &ws;

  // Firmware update route
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) { handleUpdate(request); });

  server.on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest* request) {},
      [](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data,
          size_t len, bool final) { handleDoUpdate(request, filename, index, data, len, final); });

  Update.onProgress(printUpdateProgress);

  Serial.println("OTAUpdate initialized (routes + progress callback)");
}

} // namespace OTAUpdate
