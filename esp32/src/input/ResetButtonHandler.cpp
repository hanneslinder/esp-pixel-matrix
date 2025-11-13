#include "ResetButtonHandler.h"

ResetButtonHandler::ResetButtonHandler(int pin, unsigned long shortPressThreshold)
    : _pin(pin)
    , _shortPressThreshold(shortPressThreshold)
    , _lastState(HIGH)
    , _currentState(HIGH)
    , _pressedTime(0)
    , _releasedTime(0)
    , _pressDuration(0)
    , _longPressCallback(nullptr)
{
}

void ResetButtonHandler::begin() { pinMode(_pin, INPUT_PULLUP); }

void ResetButtonHandler::update()
{
  _currentState = digitalRead(_pin);

  // Track how long the button is held down
  if (_currentState == LOW && _lastState == LOW) {
    _pressDuration++;
  }

  // Button just pressed
  if (_lastState == HIGH && _currentState == LOW) {
    _pressedTime = millis();
  }
  // Button just released
  else if (_lastState == LOW && _currentState == HIGH) {
    _releasedTime = millis();
    _pressDuration = 0;

    long pressDuration = _releasedTime - _pressedTime;

    // Check if it was a long press
    if (pressDuration > _shortPressThreshold) {
      Serial.println("RESET BUTTON PRESS");
      if (_longPressCallback != nullptr) {
        _longPressCallback();
      }
    }
  }

  _lastState = _currentState;
}

bool ResetButtonHandler::isPressed() const { return _currentState == LOW; }

unsigned long ResetButtonHandler::getPressDuration() const { return _pressDuration; }

void ResetButtonHandler::onLongPress(std::function<void()> callback)
{
  _longPressCallback = callback;
}
