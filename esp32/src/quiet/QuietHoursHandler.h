#ifndef QUIET_HOURS_HANDLER_H
#define QUIET_HOURS_HANDLER_H

#include <Arduino.h>
#include <time.h>

#include "../matrix/MatrixController.h"

/**
 * QuietHoursHandler - replaces the matrix content with a sleep icon during a
 * configured window of the day.
 *
 * The window is stored on the device (see ConfigManager) and repeats daily.
 * It is treated as a half-open interval [start, end): the matrix goes to sleep
 * at `start` and wakes at `end`. A window whose start is later than its end
 * crosses midnight, so 22:00-07:00 sleeps from 22:00 until 07:00.
 *
 * While active the panel is forced to QUIET_HOURS_BRIGHTNESS; the brightness
 * that was active just before is restored on exit.
 */
class QuietHoursHandler {
  public:
  explicit QuietHoursHandler(MatrixController& matrix);

  void begin();

  /**
   * Re-evaluates the window and draws/undraws the sleep screen as needed.
   * Cheap to call every loop: the clock is only read once per second and the
   * icon is only redrawn on a state change or roughly once a second.
   */
  void update();

  /** Re-reads the window from ConfigManager, e.g. after a WebSocket update. */
  void handleConfigChange();

  bool isActive() const { return _active; }

  private:
  void setActive(bool active);
  void drawSleepIcon();

  MatrixController& _matrix;

  bool _active;
  bool _evaluated;
  unsigned long _lastCheck;
  unsigned long _lastDraw;
  uint8_t _brightnessBeforeQuiet;
};

#endif // QUIET_HOURS_HANDLER_H
