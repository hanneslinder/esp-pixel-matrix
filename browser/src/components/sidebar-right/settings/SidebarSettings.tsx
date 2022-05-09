import { view } from "@risingstack/react-easy-state";
import React, { useEffect } from "react";
import { FirmwareUpdateForm } from "./FirmwareUpdateForm";
import { BrightnessSlider } from "./BrightnessSlider";
import { Expandable } from "../../utils/Expandable";
import { SaveLoad } from "./SaveLoad";
import { Canvas } from "../../canvas/Canvas";
import { getPixelsAction, getStateAction, resetAction } from "../../../Actions";
import { getSocket, IncommingMessageType } from "../../../Websocket";
import { StateFromRemote } from "../../../utils/storage";
import { appState } from "../../../state/appState";

import "./SidebarSettings.less";

interface Props {
	getCanvas: () => Canvas;
}

export const SidebarSettings: React.FC<Props> = view(({ getCanvas }) => {
	let unsubscribeFromStateMessage: () => void;

	const onRemoteStateReceived = (state: StateFromRemote) => {
		Object.keys(state).forEach((key: keyof StateFromRemote) => {
			(appState[key] as any) = state[key];
		});
	};

	const getRemoteState = () => {
		getPixelsAction();
		getStateAction();
	};

	const reset = () => {
		resetAction();
	};

	useEffect(() => {
		unsubscribeFromStateMessage = getSocket().subscribe(IncommingMessageType.MatrixSettingsResponse, onRemoteStateReceived);

		return function cleanup() {
			unsubscribeFromStateMessage();
		};
	}, [unsubscribeFromStateMessage]);

	return (
		<div className="sidebar-settings">
			<BrightnessSlider />
			<Expandable title="Firmware update" className="expandable-firmware" initialOpen={false}>
				<FirmwareUpdateForm />
			</Expandable>
			<SaveLoad getCanvas={getCanvas} />
			<button onClick={() => getRemoteState()}>Sync</button>
			<button onClick={() => reset()}>Reset WIFI</button>
		</div>
	);
});
