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
  _server.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=14400");

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", String(), false, processHtmlTemplate);
  });

  _server.serveStatic("/main.css", SPIFFS, "/main.css").setCacheControl("max-age=14400");
  _server.serveStatic("/main.js", SPIFFS, "/main.js").setCacheControl("max-age=14400");

  OTAUpdate::init(_server, _ws);

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
