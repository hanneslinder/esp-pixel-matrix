#include "QuietHoursHandler.h"

#include "../config/ConfigManager.h"
#include "../config/settings.h"

static ConfigManager& config = ConfigManager::getInstance();

namespace {

// "HH:MM" -> minutes since midnight. Returns -1 when unparsable.
int parseTimeOfDay(const char* value)
{
  if (value == nullptr || value[0] == '\0' || strlen(value) != 5 || value[2] != ':') {
    return -1;
  }

  const int hours = (value[0] - '0') * 10 + (value[1] - '0');
  const int minutes = (value[3] - '0') * 10 + (value[4] - '0');

  if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
    return -1;
  }

  return hours * 60 + minutes;
}

// Half-open window [start, end), where start > end means it crosses midnight.
bool isWithinWindow(int nowMinutes, int startMinutes, int endMinutes)
{
  if (startMinutes == endMinutes) {
    return false; // zero length window is never active
  }

  if (startMinutes < endMinutes) {
    return nowMinutes >= startMinutes && nowMinutes < endMinutes;
  }

  return nowMinutes >= startMinutes || nowMinutes < endMinutes;
}

} // namespace

QuietHoursHandler::QuietHoursHandler(MatrixController& matrix)
    : _matrix(matrix)
    , _active(false)
    , _evaluated(false)
    , _lastCheck(0)
    , _lastDraw(0)
    , _brightnessBeforeQuiet(MIN_BRIGHTNESS)
{
}

void QuietHoursHandler::begin()
{
  Serial.printf("Quiet hours: %s\n",
      config.hasQuietHours() ? "enabled" : "disabled (or not configured)");

  if (config.hasQuietHours()) {
    Serial.printf("Quiet hours window: %s - %s\n", config.getQuietHoursStart(),
        config.getQuietHoursEnd());
  }

  // Force a fresh evaluation on the first update().
  _evaluated = false;
  _lastCheck = 0;
}

void QuietHoursHandler::handleConfigChange()
{
  Serial.printf("Quiet hours updated: %s\n",
      config.hasQuietHours() ? "enabled" : "disabled (or not configured)");

  if (config.hasQuietHours()) {
    Serial.printf("Quiet hours window: %s - %s\n", config.getQuietHoursStart(),
        config.getQuietHoursEnd());
  }

  // Re-evaluate immediately instead of waiting for the next second tick.
  _evaluated = false;
  _lastCheck = 0;

  if (!config.hasQuietHours() && _active) {
    setActive(false);
  }
}

void QuietHoursHandler::update()
{
  const unsigned long now = millis();

  // Reading the clock (and the RTC) once a second is plenty for minute
  // granularity and keeps the loop free.
  if (_evaluated && now - _lastCheck < 1000) {
    // Still redraw occasionally so the icon survives anything that clears the
    // panel behind our back (startup banner, OTA, reset overlay).
    if (_active && now - _lastDraw > 1000) {
      drawSleepIcon();
      _matrix.render(config.getCompositionMode(), true);
      _lastDraw = now;
    }
    return;
  }

  _lastCheck = now;

  if (!config.hasQuietHours()) {
    if (_active) {
      setActive(false);
    }
    _evaluated = true;
    return;
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    // No valid time yet (NTP not synced). Leave the screen alone rather than
    // guessing: a bogus clock could otherwise put the matrix to sleep all day.
    _evaluated = true;
    return;
  }

  const int startMinutes = parseTimeOfDay(config.getQuietHoursStart());
  const int endMinutes = parseTimeOfDay(config.getQuietHoursEnd());

  if (startMinutes < 0 || endMinutes < 0) {
    if (_active) {
      setActive(false);
    }
    _evaluated = true;
    return;
  }

  const int nowMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  const bool shouldBeActive = isWithinWindow(nowMinutes, startMinutes, endMinutes);

  if (shouldBeActive != _active) {
    setActive(shouldBeActive);
  } else if (shouldBeActive && now - _lastDraw > 1000) {
    drawSleepIcon();
    _matrix.render(config.getCompositionMode(), true);
    _lastDraw = now;
  }

  _evaluated = true;
}

void QuietHoursHandler::setActive(bool active)
{
  _active = active;

  if (active) {
    Serial.println("Quiet hours started, showing sleep screen");
    // Remember what the user had before forcing the sleep brightness.
    _brightnessBeforeQuiet = _matrix.getBrightness();
    _matrix.setBrightness(QUIET_HOURS_BRIGHTNESS);
    drawSleepIcon();
    _lastDraw = millis();
  } else {
    Serial.println("Quiet hours ended, restoring the display");
    // The background layer still holds the user's drawing, so the normal
    // composition is simply resumed. Brightness goes back to whatever was
    // active before quiet hours started.
    _matrix.setBrightness(_brightnessBeforeQuiet);
  }
}

void QuietHoursHandler::drawSleepIcon()
{
  _matrix.renderQuietHours();
}
