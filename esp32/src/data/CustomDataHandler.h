#ifndef CUSTOM_DATA_HANDLER_H
#define CUSTOM_DATA_HANDLER_H

#include <Arduino.h>
#include <HTTPClient.h>

class CustomDataHandler {
public:
  CustomDataHandler();

  void setEnabled(bool enabled);
  void setUpdateInterval(int intervalSeconds);
  void setServerUrl(const char* serverUrl);

  void update();

  bool isEnabled() const;
  int getUpdateInterval() const;
  const char* getServerUrl() const;
  String getLastData() const;

private:
  String httpGETRequest(const char* serverUrl);

  bool _enabled;
  int _updateInterval; // in seconds, -1 means disabled
  char _serverUrl[128];
  unsigned long _lastUpdate;
  String _lastData;
};

#endif // CUSTOM_DATA_HANDLER_H
