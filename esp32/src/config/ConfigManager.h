#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * ConfigManager - Centralized configuration management
 *
 * Handles loading, saving, and accessing all application settings.
 * Persists configuration to SPIFFS as JSON.
 */
class ConfigManager {
  public:
  // Singleton access
  static ConfigManager& getInstance();

  // Initialization
  bool begin();
  bool load();
  bool save();
  void reset(); // Reset to defaults

  // Network Configuration
  const char* getNtpServer() const;
  void setNtpServer(const char* server);

  const char* getHostname() const;
  void setHostname(const char* hostname);

  // Time Configuration
  const char* getTimezone() const;
  void setTimezone(const char* tz);

  const char* getLocale() const;
  void setLocale(const char* locale);

  // Display Configuration
  int getBrightness() const;
  void setBrightness(int brightness);

  int getCompositionMode() const;
  void setCompositionMode(int mode);

  // Clock Configuration
  bool isClockVisible() const;
  void setClockVisible(bool visible);

  uint16_t getTimeColor() const;
  void setTimeColor(uint16_t color);

  uint16_t getDateColor() const;
  void setDateColor(uint16_t color);

  // Custom Data Configuration
  bool isCustomDataEnabled() const;
  void setCustomDataEnabled(bool enabled);

  int getCustomDataInterval() const;
  void setCustomDataInterval(int interval);

  const char* getCustomDataServer() const;
  void setCustomDataServer(const char* server);

  // Utility
  void printConfig() const; // Debug output

  private:
  ConfigManager();
  ConfigManager(const ConfigManager&) = delete;
  ConfigManager& operator=(const ConfigManager&) = delete;

  void loadDefaults();
  bool validateConfig();

  // Configuration file path
  static constexpr const char* CONFIG_FILE = "/config.json";

  // Network settings
  char _ntpServer[64];
  char _hostname[32];

  // Time settings
  char _timezone[64];
  char _locale[32];

  // Display settings
  int _brightness;
  int _compositionMode;

  // Clock settings
  bool _clockVisible;
  uint16_t _timeColor;
  uint16_t _dateColor;

  // Custom data settings
  bool _customDataEnabled;
  int _customDataInterval;
  char _customDataServer[128];

  bool _initialized;
};

#endif // CONFIG_MANAGER_H
