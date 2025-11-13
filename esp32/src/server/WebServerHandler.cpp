#include "WebServerHandler.h"
#include "../ota/OTAUpdateHandler.h"
#include "SPIFFS.h"

WebServerHandler::WebServerHandler(AsyncWebServer& server, AsyncWebSocket& ws)
    : _server(server)
    , _ws(ws)
{
}

void WebServerHandler::begin()
{
  // Serve static files from SPIFFS
  _server.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=14400");

  // Serve index.html with template processing
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", String(), false, processHtmlTemplate);
  });

  // Serve CSS and JS files
  _server.serveStatic("/main.css", SPIFFS, "/main.css").setCacheControl("max-age=14400");
  _server.serveStatic("/main.js", SPIFFS, "/main.js").setCacheControl("max-age=14400");

  // Initialize OTA update routes
  OTAUpdate::init(_server, _ws);

  // Start the server
  _server.begin();

  Serial.println("Web server started");
}

String WebServerHandler::processHtmlTemplate(const String& var)
{
  if (var == "websocketUrl") {
    const String ip = WiFi.localIP().toString();
    return ip;
  }

  return String();
}

String WebServerHandler::getIPAddress() const { return WiFi.localIP().toString(); }
