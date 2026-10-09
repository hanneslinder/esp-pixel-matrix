import { view } from "@risingstack/react-easy-state";
import { FirmwareUpdateForm } from "./FirmwareUpdateForm";
import { BrightnessSlider } from "./BrightnessSlider";
import { QuietHours } from "./QuietHours";
import { SaveLoad } from "./SaveLoad";
import { Canvas } from "../../canvas/Canvas";
import { getPixelsAction, getStateAction, resetAction } from "../../../Actions";
import { Expandable } from "../../utils/Expandable";

interface Props {
  getCanvas: () => Canvas;
}

export const SettingsView = view(({ getCanvas }: Props) => {
  const getRemoteState = () => {
    getPixelsAction();
    getStateAction();
  };

  const reset = () => {
    resetAction();
  };

  return (
    <div className="mx-5">
      <BrightnessSlider />
      <QuietHours />
      <Expandable
        expandedClassName="p-0"
        collapsedContent={<div>Firmware Update</div>}
        expandedContent={<FirmwareUpdateForm />}
        initialOpen={false}
      />
      <SaveLoad getCanvas={getCanvas} />
      <div className="flex gap-2">
        <button
          className="btn btn-sm btn-outline"
          onClick={() => getRemoteState()}
        >
          Sync
        </button>
        <button
          className="btn btn-sm btn-outline btn-error"
          onClick={() => reset()}
        >
          Reset WIFI
        </button>
      </div>
    </div>
  );
});
