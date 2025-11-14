import { view } from "@risingstack/react-easy-state";
import React, { useState, useEffect } from "react";
import { appState } from "../../state/appState";
import { stringToColor } from "../../utils/color";

import { HexColorPicker } from "react-colorful";

interface ColorPickerProps {
  onChange?: (color: string) => void;
  color?: string;
  className?: string;
}

export const ColorPicker = view((props: ColorPickerProps) => {
  const [inputValue, setInputValue] = useState(appState.tools.color);

  useEffect(() => {
    setInputValue(appState.tools.color);
  }, [appState.tools.color]);

  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = e.currentTarget.value;
    setInputValue(value);

    const color = stringToColor(value);
    if (color) {
      appState.tools.color = color;
    }
  };

  return (
    <div style={{ display: "flex", flexDirection: "column", gap: "10px" }}>
      <HexColorPicker
        color={appState.tools.color}
        onChange={(color) => (appState.tools.color = color)}
      />
      <input
        type="text"
        value={inputValue}
        className="input bg-gray-900"
        onChange={handleInputChange}
      />
    </div>
  );
});
