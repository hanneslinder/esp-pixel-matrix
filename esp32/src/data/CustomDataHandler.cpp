#include "CustomDataHandler.h"

CustomDataHandler::CustomDataHandler()
    : _enabled(false)
    , _updateInterval(-1)
    , _lastUpdate(0)
    , _lastData("{}")
{
  _serverUrl[0] = '\0';
}

void CustomDataHandler::setEnabled(bool enabled) { _enabled = enabled; }

void CustomDataHandler::setUpdateInterval(int intervalSeconds)
{
  _updateInterval = intervalSeconds;
}

void CustomDataHandler::setServerUrl(const char* serverUrl)
{
  strlcpy(_serverUrl, serverUrl, sizeof(_serverUrl));
}

void CustomDataHandler::update()
{
  // Check if custom data fetching is enabled and configured
  if (!_enabled || _updateInterval < 0 || strlen(_serverUrl) == 0) {
    return;
  }

  // Check if it's time to update (updateInterval is in seconds)
  if (millis() - _lastUpdate > static_cast<unsigned long>(_updateInterval) * 1000) {
    _lastUpdate = millis();

    Serial.println("Fetching custom data...");
    Serial.printf("URL: %s\n", _serverUrl);
    Serial.printf("Interval: %d seconds\n", _updateInterval);

    _lastData = httpGETRequest(_serverUrl);

    Serial.println("Custom data received:");
    Serial.println(_lastData);

    // TODO: Process and display the custom data
    // This could be extended to parse JSON and display specific fields
  }
}

bool CustomDataHandler::isEnabled() const { return _enabled; }

int CustomDataHandler::getUpdateInterval() const { return _updateInterval; }

const char* CustomDataHandler::getServerUrl() const { return _serverUrl; }

String CustomDataHandler::getLastData() const { return _lastData; }

String CustomDataHandler::httpGETRequest(const char* serverUrl)
{
  HTTPClient http;

  http.begin(serverUrl);
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("HTTP Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload;
}
