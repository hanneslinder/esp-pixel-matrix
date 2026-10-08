# Analog Clock Face Implementation Plan

**Date:** November 17, 2025  
**Status:** Planning Phase - Ready to Implement

## Overview

Implementation of an analog clock face feature for the 64x32 pixel LED matrix display. The clock will be rendered autonomously by the ESP32 and configured via the web UI.

## Design Decisions

### Architecture Pattern

Follows the existing `TextDisplayHandler` pattern:

- **ESP32**: Autonomously renders clock every second using system time
- **Web UI**: Sends configuration once, then provides preview
- **No repeated WebSocket messages** - ESP32 handles all rendering independently

### Feasibility Analysis

A 64x32 pixel display can show a readable analog clock:

- **Clock diameter**: ~28-30 pixels (fits within 32px height)
- **Horizontal centering**: ~17px margins on 64px width
- **Hand design**: Thick hands (2-3px) with different lengths and colors
- **Hour markers**: 12 dots or cardinal points (3, 6, 9, 12)

Similar to classic LCD watch faces that were 20-30 pixels and perfectly readable.

## Implementation Plan

### 1. Add Clock Configuration Types

#### Frontend: `browser/src/state/appState.ts`

```typescript
export interface ClockConfig {
  enabled: boolean;
  centerX: number;          // X position of clock center (0-63)
  centerY: number;          // Y position of clock center (0-31)
  radius: number;           // Clock face radius in pixels
  hourHandColor: string;    // Hex color for hour hand
  minuteHandColor: string;  // Hex color for minute hand
  secondHandColor: string;  // Hex color for second hand
  showSecondHand: boolean;  // Toggle second hand visibility
  hourMarkers: boolean;     // Show hour marker dots
  showBorder: boolean;      // Show clock circle border
  borderColor: string;      // Hex color for border
  hourHandLength: number;   // Length multiplier (0.0-1.0)
  minuteHandLength: number; // Length multiplier (0.0-1.0)
  secondHandLength: number; // Length multiplier (0.0-1.0)
  hourHandWidth: number;    // Hand thickness in pixels
  minuteHandWidth: number;  // Hand thickness in pixels
  secondHandWidth: number;  // Hand thickness in pixels
}

// Add to AppState interface
export interface AppState {
  // ... existing fields ...
  clock: ClockConfig;
}

// Add to getInitialState()
clock: {
  enabled: false,
  centerX: 32,
  centerY: 16,
  radius: 14,
  hourHandColor: "#ffffff",
  minuteHandColor: "#00aaff",
  secondHandColor: "#ff0000",
  showSecondHand: true,
  hourMarkers: true,
  showBorder: true,
  borderColor: "#ffffff",
  hourHandLength: 0.5,
  minuteHandLength: 0.8,
  secondHandLength: 0.9,
  hourHandWidth: 2,
  minuteHandWidth: 2,
  secondHandWidth: 1,
}
```

#### Backend: `esp32/src/types/CommonTypes.h`

```cpp
struct ClockConfig {
  bool enabled;
  int8_t centerX;
  int8_t centerY;
  uint8_t radius;
  uint16_t hourHandColor;     // 565 RGB color
  uint16_t minuteHandColor;   // 565 RGB color
  uint16_t secondHandColor;   // 565 RGB color
  bool showSecondHand;
  bool hourMarkers;
  bool showBorder;
  uint16_t borderColor;
  float hourHandLength;       // 0.0-1.0 multiplier
  float minuteHandLength;     // 0.0-1.0 multiplier
  float secondHandLength;     // 0.0-1.0 multiplier
  uint8_t hourHandWidth;
  uint8_t minuteHandWidth;
  uint8_t secondHandWidth;
};
```

### 2. Create ESP32 ClockDisplayHandler

#### `esp32/src/display/ClockDisplayHandler.h`

```cpp
#ifndef CLOCK_DISPLAY_HANDLER_H
#define CLOCK_DISPLAY_HANDLER_H

#include <Arduino.h>
#include <time.h>
#include "../matrix/MatrixController.h"
#include "../types/CommonTypes.h"

class ClockDisplayHandler {
public:
  ClockDisplayHandler(MatrixController& matrix, ClockConfig* config);

  void renderClock();
  ClockConfig* getConfig();

private:
  void drawClockFace();
  void drawHourMarkers();
  void drawClockHands(int hour, int minute, int second);
  void drawHand(float angle, float length, uint16_t color, uint8_t width);

  MatrixController& _matrix;
  ClockConfig* _config;
  time_t _lastUpdateTime;
};

#endif // CLOCK_DISPLAY_HANDLER_H
```

#### `esp32/src/display/ClockDisplayHandler.cpp`

Key implementation points:

- Get current time using `getLocalTime()`
- Calculate hand angles:
  - Hour: `(hour % 12) * 30 + (minute * 0.5)` degrees
  - Minute: `minute * 6` degrees
  - Second: `second * 6` degrees
- Convert degrees to radians and calculate endpoints
- Use `matrix.drawLine()` for hands
- Use `matrix.drawCircle()` for border
- Use `matrix.fillCircle()` for hour markers
- Only update once per second (check `time_t` change)

### 3. Add WebSocket Handler for Clock Config

#### `esp32/src/websocket/WebSocketHandler.cpp`

Add `handleSetClock()` function:

```cpp
void handleSetClock(JsonDocument& doc) {
  JsonObject clockObj = doc["clock"].as<JsonObject>();

  clockConfig.enabled = clockObj["enabled"];
  clockConfig.centerX = clockObj["centerX"];
  clockConfig.centerY = clockObj["centerY"];
  clockConfig.radius = clockObj["radius"];
  clockConfig.hourHandColor = strtol(clockObj["hourHandColor"], NULL, 16);
  clockConfig.minuteHandColor = strtol(clockObj["minuteHandColor"], NULL, 16);
  clockConfig.secondHandColor = strtol(clockObj["secondHandColor"], NULL, 16);
  clockConfig.showSecondHand = clockObj["showSecondHand"];
  clockConfig.hourMarkers = clockObj["hourMarkers"];
  clockConfig.showBorder = clockObj["showBorder"];
  clockConfig.borderColor = strtol(clockObj["borderColor"], NULL, 16);
  clockConfig.hourHandLength = clockObj["hourHandLength"];
  clockConfig.minuteHandLength = clockObj["minuteHandLength"];
  clockConfig.secondHandLength = clockObj["secondHandLength"];
  clockConfig.hourHandWidth = clockObj["hourHandWidth"];
  clockConfig.minuteHandWidth = clockObj["minuteHandWidth"];
  clockConfig.secondHandWidth = clockObj["secondHandWidth"];
}
```

Register in `handleMessage()`:

```cpp
} else if (isStringEqual(action, "setClock")) {
  handleSetClock(doc);
}
```

### 4. Integrate Clock Rendering in Main Loop

#### `esp32/src/main.cpp`

```cpp
// Add global
ClockConfig clockConfig = {
  .enabled = false,
  .centerX = 32,
  .centerY = 16,
  .radius = 14,
  // ... other defaults
};

// Initialize in setup()
ClockDisplayHandler clockDisplay(matrix, &clockConfig);

// In loop(), add after text rendering:
} else if (clockConfig.enabled) {
  clockDisplay.renderClock();
}
```

### 5. Create Browser Canvas Clock Preview Layer

#### `browser/src/components/canvas/CanvasClockLayer.ts`

```typescript
import { observe } from "@nx-js/observer-util";
import { appState } from "../../state/appState";

export class CanvasClockLayer {
  private ctx: CanvasRenderingContext2D;
  private canvas: HTMLCanvasElement;
  private updateInterval: any;

  constructor(container: HTMLElement) {
    this.canvas = document.createElement("canvas");
    this.canvas.setAttribute("id", "canvas-clock-layer");
    this.ctx = this.canvas.getContext("2d");

    this.canvas.width = appState.settings.width * appState.settings.pixelRatio;
    this.canvas.height =
      appState.settings.height * appState.settings.pixelRatio;

    container.appendChild(this.canvas);
    this.observeChanges();
  }

  private observeChanges() {
    observe(() => {
      if (appState.clock.enabled) {
        this.startClock();
      } else {
        this.stopClock();
      }
    });
  }

  private startClock() {
    clearInterval(this.updateInterval);
    this.updateInterval = setInterval(() => this.renderClock(), 1000);
    this.renderClock(); // Immediate render
  }

  private stopClock() {
    clearInterval(this.updateInterval);
    this.reset();
  }

  private renderClock() {
    const config = appState.clock;
    const ratio = appState.settings.pixelRatio;
    const now = new Date();

    this.reset();

    if (!config.enabled) return;

    const centerX = config.centerX * ratio;
    const centerY = config.centerY * ratio;
    const radius = config.radius * ratio;

    // Draw border
    if (config.showBorder) {
      this.ctx.strokeStyle = config.borderColor;
      this.ctx.lineWidth = ratio;
      this.ctx.beginPath();
      this.ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
      this.ctx.stroke();
    }

    // Draw hour markers
    if (config.hourMarkers) {
      for (let i = 0; i < 12; i++) {
        const angle = ((i * 30 - 90) * Math.PI) / 180;
        const markerRadius = radius * 0.85;
        const x = centerX + Math.cos(angle) * markerRadius;
        const y = centerY + Math.sin(angle) * markerRadius;

        this.ctx.fillStyle = config.borderColor;
        this.ctx.beginPath();
        this.ctx.arc(x, y, ratio, 0, 2 * Math.PI);
        this.ctx.fill();
      }
    }

    // Calculate angles
    const hours = now.getHours() % 12;
    const minutes = now.getMinutes();
    const seconds = now.getSeconds();

    const hourAngle = ((hours * 30 + minutes * 0.5 - 90) * Math.PI) / 180;
    const minuteAngle = ((minutes * 6 - 90) * Math.PI) / 180;
    const secondAngle = ((seconds * 6 - 90) * Math.PI) / 180;

    // Draw hands
    this.drawHand(
      centerX,
      centerY,
      hourAngle,
      radius * config.hourHandLength,
      config.hourHandColor,
      config.hourHandWidth * ratio
    );
    this.drawHand(
      centerX,
      centerY,
      minuteAngle,
      radius * config.minuteHandLength,
      config.minuteHandColor,
      config.minuteHandWidth * ratio
    );
    if (config.showSecondHand) {
      this.drawHand(
        centerX,
        centerY,
        secondAngle,
        radius * config.secondHandLength,
        config.secondHandColor,
        config.secondHandWidth * ratio
      );
    }
  }

  private drawHand(
    cx: number,
    cy: number,
    angle: number,
    length: number,
    color: string,
    width: number
  ) {
    this.ctx.strokeStyle = color;
    this.ctx.lineWidth = width;
    this.ctx.lineCap = "round";
    this.ctx.beginPath();
    this.ctx.moveTo(cx, cy);
    this.ctx.lineTo(
      cx + Math.cos(angle) * length,
      cy + Math.sin(angle) * length
    );
    this.ctx.stroke();
  }

  private reset() {
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
  }

  public destroy() {
    clearInterval(this.updateInterval);
    this.canvas.remove();
  }
}
```

### 6. Create Clock View Component

#### `browser/src/components/views/clock/ClockView.tsx`

UI controls needed:

- Toggle clock enabled
- Color pickers for each hand and border
- Sliders for position (centerX, centerY)
- Slider for radius
- Sliders for hand lengths
- Sliders for hand widths
- Toggles for second hand, hour markers, border

### 7. Add Clock Action and Integration

#### `browser/src/Actions.ts`

```typescript
export const setClockAction = () => {
  const clockToSend = {
    ...appState.clock,
    hourHandColor: convertHexTo16Bit(appState.clock.hourHandColor),
    minuteHandColor: convertHexTo16Bit(appState.clock.minuteHandColor),
    secondHandColor: convertHexTo16Bit(appState.clock.secondHandColor),
    borderColor: convertHexTo16Bit(appState.clock.borderColor),
  };

  const msg = {
    action: "setClock",
    clock: clockToSend,
  };

  socket.send(msg);
};
```

#### `browser/src/components/Navigation.tsx`

Add Clock to Views enum and navigation items

#### `browser/src/components/canvas/CanvasWrapper.tsx`

Instantiate CanvasClockLayer alongside other layers

## Technical Notes

### Math for Clock Hands

```
Hour Hand Angle = (hours % 12) * 30 + (minutes * 0.5) - 90 degrees
Minute Hand Angle = minutes * 6 - 90 degrees
Second Hand Angle = seconds * 6 - 90 degrees

// Convert to radians and calculate endpoint:
x = centerX + cos(angle * PI / 180) * length
y = centerY + sin(angle * PI / 180) * length
```

### ESP32 Drawing Functions Available

- `matrix->drawLine(x0, y0, x1, y1, color)`
- `matrix->drawCircle(x, y, radius, color)`
- `matrix->fillCircle(x, y, radius, color)`
- `matrix->drawPixel(x, y, color)`

### Performance Considerations

- ESP32 should only redraw when second changes (check `time_t`)
- Browser can update every second with `setInterval`
- Use separate layer to avoid redrawing background
- Consider clearing only clock layer, not entire display

## Testing Checklist

- [ ] Clock configuration saves/loads properly
- [ ] Clock hands move correctly (test at different times)
- [ ] Browser preview matches ESP32 display
- [ ] Second hand toggle works
- [ ] Hour markers toggle works
- [ ] All color pickers work
- [ ] Clock can be positioned anywhere on display
- [ ] Clock resizes correctly with radius slider
- [ ] Hand lengths and widths adjust properly
- [ ] Clock disables properly when toggled off
- [ ] Clock works with different composition modes
- [ ] Clock coexists with text/background layers

## Future Enhancements (Optional)

- Multiple clock face styles (Roman numerals, digital hybrid, etc.)
- Date display inside/below clock
- Timezone selection per clock
- Multiple clocks showing different timezones
- Smooth second hand motion (interpolation)
- Custom color schemes / presets
- Day/night mode auto-brightness
