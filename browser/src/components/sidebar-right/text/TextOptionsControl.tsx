import { view } from "@risingstack/react-easy-state";
import React from "react";
import { HexColorPicker } from "react-colorful";
import { setTextAction } from "../../../Actions";
import { appState, TextAlign, TextOptions } from "../../../state/appState";
import { limitNumberBetween } from "../../../utils/utils";
import { Dropdown, DropdownItem } from "../../utils/DropDown";
import { Expandable } from "../../utils/Expandable";
import { SvgIcon } from "../../utils/SvgIcon";

const iconHelp = require("../../../assets/info-circle.svg");
const iconRemove = require("../../../assets/x-circle.svg");

const textAlignItems: DropdownItem[] = [
	{ label: "Left", value: TextAlign.LEFT },
	{ label: "Center", value: TextAlign.CENTER },
	{ label: "Right", value: TextAlign.RIGHT },
];

interface Props {
	settings: TextOptions;
	onResize?: () => void;
}

let colorChangeTimeout: ReturnType<typeof setTimeout>;
const COLORPICKER_TIMEOUT_MS = 350;

export const TextOptionsControl: React.FC<Props> = view(({ settings, onResize }) => {
	const onTextChange = (text: string) => {
		const newSettings = { ...settings, text };
		updateTextItem(newSettings);
	};

	const onXOffsetChange = (offset: number) => {
		const offsetX = limitNumberBetween(offset, -20, 20);
		const newSettings = { ...settings, offsetX };
		updateTextItem(newSettings);
	};

	const onYOffsetChange = (offset: number) => {
		const offsetY = limitNumberBetween(offset, -20, 20);
		const newSettings = { ...settings, offsetY };
		updateTextItem(newSettings);
	};

	const changeSize = (size: number) => {
		const newSettings = { ...settings, size };
		updateTextItem(newSettings);
	};

	const onAlignChange = (align: TextAlign) => {
		const newSettings = { ...settings, align };
		updateTextItem(newSettings);
	};

	const onColorChange = (color: string) => {
		clearTimeout(colorChangeTimeout);
		colorChangeTimeout = setTimeout(() => {
			const newSettings = { ...settings, color };
			updateTextItem(newSettings);
		}, COLORPICKER_TIMEOUT_MS);
	};

	const removeTextItem = (line: number) => {
		appState.text = appState.text.filter((t) => t.line !== line);
		setTextAction();
	};

	const updateTextItem = (newText: TextOptions) => {
		const index = appState.text.findIndex((t) => t.line === newText.line);

		if (index > -1) {
			appState.text[index] = newText;
			setTextAction();
		}
	};

	const updateHeightOnAnimationEnd = () => {
		setTimeout( () => {
			onResize();
		}, COLORPICKER_TIMEOUT_MS);
	}

	const clorpickerPreview = <div className="color-picker-preview" style={{ background: settings.color }} />;

	return (
		<div className="sidebar-settings-section">
			<div className="sidebar-settings-item">
				<div className="sidebar-settings-label">
					<span>Text</span>
					<SvgIcon
						className="icon-text-help"
						icon={iconHelp}
						title="help"
						onClick={() => window.open("https://www.cplusplus.com/reference/ctime/strftime/", "_blank")}
					/>
				</div>
				<div className="sidebar-settings-input">
					<input type="text" value={settings.text} onChange={(e) => onTextChange(e.target.value)} />
				</div>
			</div>
			<div className="sidebar-settings-item">
				<div className="sidebar-settings-label">Size</div>
				<div className="sidebar-settings-input">
					<div className="offset-input">{settings.size}</div>
					<div className="offset-buttons">
						<div
							className={`offset-icon decrement ${settings.size === 1 ? "is-disabled" : ""}`}
							onClick={() => changeSize(settings.size - 1)}
						>
							-
						</div>
						<div
							className={`offset-icon increment ${settings.size > 4 ? "is-disabled" : ""}`}
							onClick={() => changeSize(settings.size + 1)}
						>
							+
						</div>
					</div>
				</div>
			</div>
			<div className="sidebar-settings-item">
				<div className="sidebar-settings-label">Align</div>
				<div className="sidebar-settings-input">
					<Dropdown selected={settings.align} items={textAlignItems} onSelect={onAlignChange} />
				</div>
			</div>
			<div className="sidebar-settings-item">
				<div className="sidebar-settings-label">Offset X</div>
				<div className="sidebar-settings-input">
					<div className="offset-input">{settings.offsetX}</div>
					<div className="offset-buttons">
						<div
							className={`offset-icon decrement ${settings.offsetX < -19 ? "is-disabled" : ""}`}
							onClick={() => onXOffsetChange(settings.offsetX - 1)}
						>
							-
						</div>
						<div
							className={`offset-icon increment ${settings.offsetX > 19 ? "is-disabled" : ""}`}
							onClick={() => onXOffsetChange(settings.offsetX + 1)}
						>
							+
						</div>
					</div>
				</div>
			</div>
			<div className="sidebar-settings-item">
				<div className="sidebar-settings-label">Offset Y</div>
				<div className="sidebar-settings-input">
					<div className="settings-input">{settings.offsetY}</div>
					<div className="offset-buttons">
						<div
							className={`offset-icon decrement ${settings.offsetY < -19 ? "is-disabled" : ""}`}
							onClick={() => onYOffsetChange(settings.offsetY - 1)}
						>
							-
						</div>
						<div
							className={`offset-icon increment ${settings.offsetY > 19 ? "is-disabled" : ""}`}
							onClick={() => onYOffsetChange(settings.offsetY + 1)}
						>
							+
						</div>
					</div>
				</div>
			</div>
			<div className="sidebar-settings-item settings-color">
				<Expandable title="Color" initialOpen={false} titleElement={clorpickerPreview} onExpand={updateHeightOnAnimationEnd}>
					<HexColorPicker color={settings.color} onChange={(color) => onColorChange(color)} />
				</Expandable>
			</div>
			<div className="btn-delete-text">
				<button onClick={() => removeTextItem(settings.line)}>delete</button>
			</div>
		</div>
	);
});
