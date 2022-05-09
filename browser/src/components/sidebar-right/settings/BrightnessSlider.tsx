import React, { ChangeEvent } from "react";
import { view } from "@risingstack/react-easy-state";
import { appState } from "../../../state/appState";
import { setBrightnessAction } from "../../../Actions";
import { SvgIcon } from "../../utils/SvgIcon";

const iconBrightnessLow = require("../../../assets/brightness-low.svg");
const iconBrightnessHigh = require("../../../assets/brightness-high.svg");

interface Props {}

export const BrightnessSlider: React.FC<Props> = view(() => {
	const onBrightnessChange = (e: ChangeEvent<HTMLInputElement>) => {
		setBrightnessAction(parseInt(e.target.value));
	};

	return (
		<div className="settings-item brightness">
			<div className="settings-item-label">Brightness</div>
			<div className="settings-item-value">
				<SvgIcon icon={appState.settings.brightness > 7 ? iconBrightnessHigh : iconBrightnessLow} />
				<input type="range" min="1" max="15" step="1.0" value={appState.settings.brightness} onChange={onBrightnessChange} />
				<input type="number" min="1" max="15" value={appState.settings.brightness} onChange={onBrightnessChange} />
			</div>
		</div>
	);
});
