#ifndef RESET_BUTTON_HANDLER_H
#define RESET_BUTTON_HANDLER_H

#include <Arduino.h>
#include <functional>

class ResetButtonHandler {
  public:
  ResetButtonHandler(int pin, unsigned long shortPressThreshold);

  void begin();
  void update();

  bool isPressed() const;
  unsigned long getPressDuration() const;

  // Callback when button is released after a long press
  void onLongPress(std::function<void()> callback);

  private:
  int _pin;
  unsigned long _shortPressThreshold;

  int _lastState;
  int _currentState;
  unsigned long _pressedTime;
  unsigned long _releasedTime;
  unsigned long _pressDuration;

  std::function<void()> _longPressCallback;
};

#endif // RESET_BUTTON_HANDLER_H
