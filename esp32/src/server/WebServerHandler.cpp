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
  // The HTML is templated - the current IP is injected per request - so it must
  // not be served through serveStatic(): that returns the raw file with the
  // "%websocketUrl%" placeholder still in it, and the web UI would then try to
  // open ws://%websocketUrl%/ws. Registering both paths keeps a direct visit to
  // /index.html working.
  auto serveIndexHtml = [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", String(), false, processHtmlTemplate);
  };

  _server.on("/", HTTP_GET, serveIndexHtml);
  _server.on("/index.html", HTTP_GET, serveIndexHtml);

  // Static assets. AsyncStaticWebHandler serves the ".gz" sibling when it exists
  // (built by browser/copyArtifact.sh) and sets Content-Encoding: gzip itself.
  _server.serveStatic("/index.css", SPIFFS, "/index.css").setCacheControl("max-age=14400");
  _server.serveStatic("/index.js", SPIFFS, "/index.js").setCacheControl("max-age=14400");

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
