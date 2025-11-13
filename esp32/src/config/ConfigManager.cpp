#include "ConfigManager.h"
#include "SPIFFS.h"
#include "settings.h"

ConfigManager& ConfigManager::getInstance()
{
  static ConfigManager instance;
  return instance;
}

ConfigManager::ConfigManager()
    : _brightness(DEFAULT_BRIGHTNESS)
    , _compositionMode(0)
    , _clockVisible(true)
    , _timeColor(0xFFFF)
    , _dateColor(0xFFFF)
    , _customDataEnabled(false)
    , _customDataInterval(-1)
    , _initialized(false)
{
  loadDefaults();
}

bool ConfigManager::begin()
{
  if (_initialized) {
    return true;
  }

  // Try to load configuration from file
  if (!load()) {
    Serial.println("No config file found or load failed, using defaults");
    loadDefaults();
  }

  _initialized = true;
  Serial.println("ConfigManager initialized");
  printConfig();

  return true;
}

void ConfigManager::loadDefaults()
{
  // Network defaults
  strlcpy(_ntpServer, "at.pool.ntp.org", sizeof(_ntpServer));
  strlcpy(_hostname, "pixelclock", sizeof(_hostname));

  // Time defaults
  strlcpy(_timezone, "CET-1CEST,M3.5.0,M10.5.0/3", sizeof(_timezone));
  strlcpy(_locale, "en_US.UTF-8", sizeof(_locale));

  // Display defaults
  _brightness = DEFAULT_BRIGHTNESS;
  _compositionMode = 0;

  // Clock defaults
  _clockVisible = true;
  _timeColor = 0xFFFF;
  _dateColor = 0xFFFF;

  // Custom data defaults
  _customDataEnabled = false;
  _customDataInterval = -1;
  _customDataServer[0] = '\0';

  Serial.println("Loaded default configuration");
}

bool ConfigManager::load()
{
  if (!SPIFFS.exists(CONFIG_FILE)) {
    Serial.println("Config file does not exist");
    return false;
  }

  File file = SPIFFS.open(CONFIG_FILE, "r");
  if (!file) {
    Serial.println("Failed to open config file for reading");
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.printf("Failed to parse config file: %s\n", error.c_str());
    return false;
  }

  // Load network settings
  if (doc["network"]["ntpServer"]) {
    strlcpy(_ntpServer, doc["network"]["ntpServer"], sizeof(_ntpServer));
  }
  if (doc["network"]["hostname"]) {
    strlcpy(_hostname, doc["network"]["hostname"], sizeof(_hostname));
  }

  // Load time settings
  if (doc["time"]["timezone"]) {
    strlcpy(_timezone, doc["time"]["timezone"], sizeof(_timezone));
  }
  if (doc["time"]["locale"]) {
    strlcpy(_locale, doc["time"]["locale"], sizeof(_locale));
  }

  // Load display settings
  if (doc["display"]["brightness"]) {
    _brightness = doc["display"]["brightness"];
  }
  if (doc["display"]["compositionMode"]) {
    _compositionMode = doc["display"]["compositionMode"];
  }

  // Load clock settings
  if (doc["clock"]["visible"]) {
    _clockVisible = doc["clock"]["visible"];
  }
  if (doc["clock"]["timeColor"]) {
    _timeColor = doc["clock"]["timeColor"];
  }
  if (doc["clock"]["dateColor"]) {
    _dateColor = doc["clock"]["dateColor"];
  }

  // Load custom data settings
  if (doc["customData"]["enabled"]) {
    _customDataEnabled = doc["customData"]["enabled"];
  }
  if (doc["customData"]["interval"]) {
    _customDataInterval = doc["customData"]["interval"];
  }
  if (doc["customData"]["server"]) {
    strlcpy(_customDataServer, doc["customData"]["server"], sizeof(_customDataServer));
  }

  Serial.println("Configuration loaded from file");
  return validateConfig();
}

bool ConfigManager::save()
{
  JsonDocument doc;

  // Network settings
  doc["network"]["ntpServer"] = _ntpServer;
  doc["network"]["hostname"] = _hostname;

  // Time settings
  doc["time"]["timezone"] = _timezone;
  doc["time"]["locale"] = _locale;

  // Display settings
  doc["display"]["brightness"] = _brightness;
  doc["display"]["compositionMode"] = _compositionMode;

  // Clock settings
  doc["clock"]["visible"] = _clockVisible;
  doc["clock"]["timeColor"] = _timeColor;
  doc["clock"]["dateColor"] = _dateColor;

  // Custom data settings
  doc["customData"]["enabled"] = _customDataEnabled;
  doc["customData"]["interval"] = _customDataInterval;
  doc["customData"]["server"] = _customDataServer;

  File file = SPIFFS.open(CONFIG_FILE, "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write config to file");
    file.close();
    return false;
  }

  file.close();
  Serial.println("Configuration saved to file");
  return true;
}

void ConfigManager::reset()
{
  loadDefaults();
  save();
  Serial.println("Configuration reset to defaults");
}

bool ConfigManager::validateConfig()
{
  bool valid = true;

  // Validate brightness
  if (_brightness < MIN_BRIGHTNESS || _brightness > MAX_BRIGHTNESS) {
    Serial.printf("Invalid brightness %d, clamping to range [%d, %d]\n", _brightness,
        MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    _brightness = constrain(_brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    valid = false;
  }

  // Validate composition mode (0-2 typically)
  if (_compositionMode < 0 || _compositionMode > 2) {
    Serial.printf("Invalid composition mode %d, resetting to 0\n", _compositionMode);
    _compositionMode = 0;
    valid = false;
  }

  // Validate custom data interval
  if (_customDataInterval < -1) {
    Serial.printf("Invalid custom data interval %d, resetting to -1\n", _customDataInterval);
    _customDataInterval = -1;
    valid = false;
  }

  return valid;
}

// Getters
const char* ConfigManager::getNtpServer() const { return _ntpServer; }
const char* ConfigManager::getHostname() const { return _hostname; }
const char* ConfigManager::getTimezone() const { return _timezone; }
const char* ConfigManager::getLocale() const { return _locale; }
int ConfigManager::getBrightness() const { return _brightness; }
int ConfigManager::getCompositionMode() const { return _compositionMode; }
bool ConfigManager::isClockVisible() const { return _clockVisible; }
uint16_t ConfigManager::getTimeColor() const { return _timeColor; }
uint16_t ConfigManager::getDateColor() const { return _dateColor; }
bool ConfigManager::isCustomDataEnabled() const { return _customDataEnabled; }
int ConfigManager::getCustomDataInterval() const { return _customDataInterval; }
const char* ConfigManager::getCustomDataServer() const { return _customDataServer; }

// Setters
void ConfigManager::setNtpServer(const char* server)
{
  strlcpy(_ntpServer, server, sizeof(_ntpServer));
}

void ConfigManager::setHostname(const char* hostname)
{
  strlcpy(_hostname, hostname, sizeof(_hostname));
}

void ConfigManager::setTimezone(const char* tz) { strlcpy(_timezone, tz, sizeof(_timezone)); }

void ConfigManager::setLocale(const char* locale) { strlcpy(_locale, locale, sizeof(_locale)); }

void ConfigManager::setBrightness(int brightness)
{
  _brightness = constrain(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
}

void ConfigManager::setCompositionMode(int mode) { _compositionMode = mode; }

void ConfigManager::setClockVisible(bool visible) { _clockVisible = visible; }

void ConfigManager::setTimeColor(uint16_t color) { _timeColor = color; }

void ConfigManager::setDateColor(uint16_t color) { _dateColor = color; }

void ConfigManager::setCustomDataEnabled(bool enabled) { _customDataEnabled = enabled; }

void ConfigManager::setCustomDataInterval(int interval) { _customDataInterval = interval; }

void ConfigManager::setCustomDataServer(const char* server)
{
  strlcpy(_customDataServer, server, sizeof(_customDataServer));
}

void ConfigManager::printConfig() const
{
  Serial.println("=== Configuration ===");
  Serial.printf("NTP Server: %s\n", _ntpServer);
  Serial.printf("Hostname: %s\n", _hostname);
  Serial.printf("Timezone: %s\n", _timezone);
  Serial.printf("Locale: %s\n", _locale);
  Serial.printf("Brightness: %d\n", _brightness);
  Serial.printf("Composition Mode: %d\n", _compositionMode);
  Serial.printf("Clock Visible: %s\n", _clockVisible ? "true" : "false");
  Serial.printf("Time Color: 0x%04X\n", _timeColor);
  Serial.printf("Date Color: 0x%04X\n", _dateColor);
  Serial.printf("Custom Data Enabled: %s\n", _customDataEnabled ? "true" : "false");
  Serial.printf("Custom Data Interval: %d\n", _customDataInterval);
  Serial.printf("Custom Data Server: %s\n", _customDataServer);
  Serial.println("=====================");
}
