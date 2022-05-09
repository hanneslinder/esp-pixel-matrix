import { view } from "@risingstack/react-easy-state";
import React, { ChangeEvent } from "react";
import { setCompositionModeAction, setTextAction } from "../../../Actions";
import { appState, TextAlign, TextOptions } from "../../../state/appState";
import { SvgIcon } from "../../utils/SvgIcon";
import { TextOptionsControl } from "./TextOptionsControl";

import "./SidebarText.less";
import { uuidv4 } from "../../../utils/utils";
import { Expandable } from "../../utils/Expandable";

const iconBlendMode1 = require("../../../assets/blend3.svg");
const iconBlendMode2 = require("../../../assets/blend2.svg");
const iconBlendMode3 = require("../../../assets/blend1.svg");
const iconPlus = require("../../../assets/plus-circle.svg");
const blendModeIcons = [iconBlendMode1, iconBlendMode2, iconBlendMode3];

interface Props {}

export const SidebarText: React.FC<Props> = view(() => {
	const setCompositionMode = () => {
		const currentMode = appState.settings.compositionMode;
		let newMode = currentMode + 1;
		setCompositionModeAction(newMode);
	};

	const renderTextSettings = () => {
		return appState.text.map((text, i) => (
			<Expandable title={`Text ${i + 1}`} initialOpen={true} key={text.id}>
				<TextOptionsControl settings={text} />
			</Expandable>
		));
	};

	const addTextItem = () => {
		const line = 1;
		const textItem: TextOptions = {
			align: TextAlign.CENTER,
			color: "#ffffff",
			text: "",
			size: 1,
			line,
			offsetX: 3,
			offsetY: 1,
			id: uuidv4(),
		};

		appState.text.push(textItem);
		setTextAction();
	};

	const renderAddTextButton = () => {
		if (appState.text.length < 5) {
			return (
				<div className="btn-add-text">
					<SvgIcon icon={iconPlus} onClick={addTextItem} />
				</div>
			);
		}
	};

	return (
		<div className="sidebar-text">
			<div className="sidebar-settings-section">
				<div className="sidebar-settings-item">
					<div className="sidebar-settings-label">Blend Mode</div>
					<div className="sidebar-settings-input">
						<SvgIcon icon={blendModeIcons[appState.settings.compositionMode]} onClick={setCompositionMode} />
					</div>
				</div>
			</div>
			{renderTextSettings()}
			{renderAddTextButton()}
		</div>
	);
});
