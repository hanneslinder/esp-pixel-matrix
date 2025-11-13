#include "WiFiConnectionHandler.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>

// External declarations for portal credentials
extern const char* portalSsid;
extern const char* portalPassword;

WiFiConnectionHandler::WiFiConnectionHandler(AsyncWebServer& server)
    : _server(server)
    , _espConnect(nullptr)
    , _useCaptivePortal(false)
    , _ssid(nullptr)
    , _password(nullptr)
    , _lastWiFiCheck(0)
    , _wifiReconnectAttempts(0)
    , _wifiReconnectStartTime(0)
{
}

WiFiConnectionHandler::~WiFiConnectionHandler()
{
  if (_espConnect != nullptr) {
    delete _espConnect;
    _espConnect = nullptr;
  }
}

void WiFiConnectionHandler::begin(bool useCaptivePortal, const char* ssid, const char* password)
{
  _useCaptivePortal = useCaptivePortal;
  _ssid = ssid;
  _password = password;

  if (_useCaptivePortal) {
    initCaptivePortal();
  } else {
    connectToWiFi(_ssid, _password);
  }
}

void WiFiConnectionHandler::loop()
{
  if (_useCaptivePortal && _espConnect != nullptr) {
    _espConnect->loop();
  }
}

void WiFiConnectionHandler::connectToWiFi(const char* ssid, const char* password)
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

void WiFiConnectionHandler::initCaptivePortal()
{
  // Initialize ESPConnect if not already done
  if (_espConnect == nullptr) {
    _espConnect = new Mycila::ESPConnect(_server);
  }

  _server.on("/clear", HTTP_GET, [this](AsyncWebServerRequest* request) {
    _espConnect->clearConfiguration();
    request->send(200);
    ESP.restart();
  });

  // network state listener
  _espConnect->listen(
      [](__unused Mycila::ESPConnect::State previous, __unused Mycila::ESPConnect::State state) {
        JsonDocument doc;
        // Note: We need to access the espConnect instance here
        // This is safe because the lambda captures the this pointer implicitly
        serializeJsonPretty(doc, Serial);
        Serial.println();
      });

  _espConnect->setAutoRestart(true);
  _espConnect->setBlocking(true);

  Serial.println("Trying to connect to saved WiFi or will start portal...");

  _espConnect->begin("arduino", portalSsid, portalPassword);

  Serial.println("ESPConnect completed, continuing setup...");
}

void WiFiConnectionHandler::checkConnection()
{
  if (millis() - _lastWiFiCheck < WIFI_CHECK_INTERVAL) {
    return;
  }

  _lastWiFiCheck = millis();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting reconnection...");

    if (_wifiReconnectAttempts == 0) {
      _wifiReconnectStartTime = millis();
    }

    _wifiReconnectAttempts++;
    Serial.printf(
        "Reconnection attempt %d/%d\n", _wifiReconnectAttempts, MAX_WIFI_RECONNECT_ATTEMPTS);

    if (_wifiReconnectAttempts >= MAX_WIFI_RECONNECT_ATTEMPTS) {
      // If multiple reconnect attempts failed, try full re-initialization
      Serial.println(
          "Multiple reconnect attempts failed, performing full WiFi re-initialization...");
      WiFi.disconnect(true);
      delay(1000);
      WiFi.mode(WIFI_STA);

      if (_useCaptivePortal) {
        Serial.println("Restarting ESP to reinitialize captive portal...");
        ESP.restart();
      } else {
        WiFi.begin(_ssid, _password);
      }

      _wifiReconnectAttempts = 0;
      _wifiReconnectStartTime = 0;
    } else {
      // Try simple reconnect first
      WiFi.disconnect();
      delay(100);
      WiFi.reconnect();
    }
  } else {
    // WiFi is connected, reset reconnect counter
    if (_wifiReconnectAttempts > 0) {
      Serial.printf("WiFi reconnected successfully after %d attempts (took %lu ms)\n",
          _wifiReconnectAttempts, millis() - _wifiReconnectStartTime);
      _wifiReconnectAttempts = 0;
      _wifiReconnectStartTime = 0;
    }
  }
}

void WiFiConnectionHandler::reset()
{
  Serial.println("Reset WIFI settings!");

  delay(1000);
  ESP.restart();
  delay(1000);
}

bool WiFiConnectionHandler::isConnected() const { return WiFi.status() == WL_CONNECTED; }

String WiFiConnectionHandler::getIPAddress() const { return WiFi.localIP().toString(); }

int WiFiConnectionHandler::getRSSI() const
{
  return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
}
