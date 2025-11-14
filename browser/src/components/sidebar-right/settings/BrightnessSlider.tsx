import { view } from "@risingstack/react-easy-state";
import { appState } from "../../../state/appState";
import { setBrightnessAction } from "../../../Actions";

import { SunDim, Sun } from "lucide-react";
import { clampValue } from "../../../utils/utils";

interface Props {}

export const BrightnessSlider: React.FC<Props> = view(() => {
  const onBrightnessChange = (e) => {
    const value = clampValue(parseInt(e.target.value), 1, 15);
    setBrightnessAction(value);
  };

  return (
    <div className="border-b border-gray-700 pb-5 mb-5">
      <div className="mb-2">Brightness</div>
      <div className="flex items-center">
        <div className="w-8 flex items-center justify-center">
          {appState.settings.brightness > 7 ? <Sun /> : <SunDim />}
        </div>
        <input
          type="range"
          min="1"
          max="15"
          step="1.0"
          value={appState.settings.brightness}
          onChange={onBrightnessChange}
          className="range range-sm flex-grow mx-5 my-0"
        />
        <input
          type="number"
          min="1"
          max="15"
          value={appState.settings.brightness}
          onChange={onBrightnessChange}
          className="input bg-gray-900 w-10 text-center"
        />
      </div>
    </div>
  );
});
