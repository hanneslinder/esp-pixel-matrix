import { view } from "@risingstack/react-easy-state";
import React, { useEffect } from "react";
import { FirmwareUpdateForm } from "./FirmwareUpdateForm";
import { BrightnessSlider } from "./BrightnessSlider";
import { SaveLoad } from "./SaveLoad";
import { Canvas } from "../../canvas/Canvas";
import { getPixelsAction, getStateAction, resetAction } from "../../../Actions";
import { getSocket, IncommingMessageType } from "../../../Websocket";
import { StateFromRemote } from "../../../utils/storage";
import { appState } from "../../../state/appState";
import { Expandable } from "../../utils/Expandable";

interface Props {
  getCanvas: () => Canvas;
}

// Sync settings from esp32 back to app
const onRemoteStateReceived = (state: StateFromRemote) => {
  Object.keys(state).forEach((key: keyof StateFromRemote) => {
    (appState[key] as any) = state[key];
    console.log("Updated appState from remote:", key, state[key]);
  });
};

export const SidebarSettings: React.FC<Props> = view(({ getCanvas }) => {
  let unsubscribeFromStateMessage: () => void;

  const getRemoteState = () => {
    getPixelsAction();
    getStateAction();
  };

  const reset = () => {
    resetAction();
  };

  useEffect(() => {
    unsubscribeFromStateMessage = getSocket().subscribe(
      IncommingMessageType.MatrixSettingsResponse,
      onRemoteStateReceived
    );

    return function cleanup() {
      unsubscribeFromStateMessage();
    };
  }, [unsubscribeFromStateMessage]);

  return (
    <div className="mx-5">
      <BrightnessSlider />
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
