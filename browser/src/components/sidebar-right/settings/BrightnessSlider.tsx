import { view } from "@risingstack/react-easy-state";
import { appState } from "../../../state/appState";
import { setBrightnessAction } from "../../../Actions";

import { SunDim, Sun } from "lucide-react";

interface Props {}

export const BrightnessSlider: React.FC<Props> = view(() => {
  const onBrightnessChange = (e) => {
    setBrightnessAction(parseInt(e.target.value));
  };

  return (
    <div className="border-b border-[--color-dark-1] pb-5 mb-5">
      <div className="mb-2.5">Brightness</div>
      <div className="flex items-center [&>svg]:translate-y-1.5">
        {appState.settings.brightness > 7 ? <Sun /> : <SunDim />}
        <input
          type="range"
          min="1"
          max="15"
          step="1.0"
          value={appState.settings.brightness}
          onChange={onBrightnessChange}
          className="flex-grow mx-5 my-0"
        />
        <input
          type="number"
          min="1"
          max="15"
          value={appState.settings.brightness}
          onChange={onBrightnessChange}
          className="bg-[--color-dark-1] text-[--color-gray-0] outline-none border-none p-1.5 w-[30px] h-[30px] text-center rounded-md"
        />
      </div>
    </div>
  );
});
