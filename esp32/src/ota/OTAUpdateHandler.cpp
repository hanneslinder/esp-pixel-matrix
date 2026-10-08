#include "OTAUpdateHandler.h"

#include <Update.h>

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
  const char* html = "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input "
                     "type='file' name='update'><input type='submit' value='Update'></form>";
  request->send(200, "text/html", html);
}

/**
 * Decide which partition an uploaded OTA file targets.
 *
 * This is matched on the filename, so it has to recognise every name the file
 * system image is published under: PlatformIO names it after
 * `board_build.filesystem` (littlefs), while the release channel and older
 * devices still ask for `spiffs.bin`. Getting this wrong writes the file system
 * image into the application partition, so match the known names exactly rather
 * than guessing at a suffix.
 */
static bool isFileSystemImage(const String& filename)
{
  String lower = filename;
  lower.toLowerCase();

  return lower.indexOf("littlefs") > -1 || lower.indexOf("spiffs") > -1
      || lower.indexOf("filesystem") > -1;
}

static void handleDoUpdate(AsyncWebServerRequest* request, const String& filename, size_t index,
    uint8_t* data, size_t len, bool final)
{
  if (!index) {
    Serial.println("Update");
    g_updateContentLength = request->contentLength();
    const bool isFileSystem = isFileSystemImage(filename);
    int cmd = isFileSystem ? U_PART : U_FLASH;

    Serial.printf("OTA target: %s (filename '%s')\n", isFileSystem ? "filesystem" : "firmware",
        filename.c_str());

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
      sendProgress(100);
      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

static void printUpdateProgress(size_t prg, size_t sz)
{
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

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) { handleUpdate(request); });

  server.on(
      "/doUpdate", HTTP_POST, [](AsyncWebServerRequest* request) {},
      [](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data,
          size_t len, bool final) { handleDoUpdate(request, filename, index, data, len, final); });

  Update.onProgress(printUpdateProgress);

  Serial.println("OTAUpdate initialized (routes + progress callback)");
}

} // namespace OTAUpdate
