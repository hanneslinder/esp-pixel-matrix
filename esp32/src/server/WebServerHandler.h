#ifndef WEB_SERVER_HANDLER_H
#define WEB_SERVER_HANDLER_H

#include <ESPAsyncWebServer.h>
#include <WiFi.h>

class WebServerHandler {
  public:
  WebServerHandler(AsyncWebServer& server, AsyncWebSocket& ws);

  void begin();
  String getIPAddress() const;

  private:
  static String processHtmlTemplate(const String& var);

  AsyncWebServer& _server;
  AsyncWebSocket& _ws;
};

#endif // WEB_SERVER_HANDLER_H
