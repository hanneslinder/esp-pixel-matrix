#ifndef WIFI_CONNECTION_HANDLER_H
#define WIFI_CONNECTION_HANDLER_H

#include <ESPAsyncWebServer.h>
#include <MycilaESPConnect.h>
#include <WiFi.h>

class WiFiConnectionHandler {
  public:
  WiFiConnectionHandler(AsyncWebServer& server);
  ~WiFiConnectionHandler();

  void begin(bool useCaptivePortal, const char* ssid = nullptr, const char* password = nullptr);
  void loop();
  void checkConnection();
  void reset();

  bool isConnected() const;
  String getIPAddress() const;
  int getRSSI() const;

  private:
  void connectToWiFi(const char* ssid, const char* password);
  void initCaptivePortal();

  AsyncWebServer& _server;
  Mycila::ESPConnect* _espConnect;

  bool _useCaptivePortal;
  const char* _ssid;
  const char* _password;

  unsigned long _lastWiFiCheck;
  int _wifiReconnectAttempts;
  unsigned long _wifiReconnectStartTime;

  static const int MAX_WIFI_RECONNECT_ATTEMPTS = 5;
  static const unsigned long WIFI_CHECK_INTERVAL = 30000; // 30 seconds
};

#endif // WIFI_CONNECTION_HANDLER_H
