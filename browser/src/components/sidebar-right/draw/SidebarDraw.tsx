import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../../state/appState";
import { GradientColorPicker } from "../../utils/GradientColorPicker";
import { ColorPicker } from "../../utils/ColorPicker";
import { SvgIcon } from "../../utils/SvgIcon";
import { Tools } from "../SidebarRight";
import { Canvas } from "../../canvas/Canvas";

import "./SidebarDraw.less";

const iconBrush = require("../../../assets/brush.svg");
const iconEraser = require("../../../assets/eraser-fill.svg");
const iconBucket = require("../../../assets/paint-bucket.svg");
const iconGradient = require("../../../assets/gradient.svg");
const iconClear = require("../../../assets/trash.svg");

interface Props {
	getCanvas: () => Canvas;
}

export const SidebarDraw: React.FC<Props> = view(({ getCanvas }) => {
	const renderColorPicker = () =>
		appState.tools.selected === Tools.GRADIENT ? (
			<GradientColorPicker
				onColor1Select={(color) => (appState.tools.gradientColor1 = color)}
				onColor2Select={(color) => (appState.tools.gradientColor2 = color)}
				color1={appState.tools.gradientColor1}
				color2={appState.tools.gradientColor2}
			/>
		) : (
			<ColorPicker />
		);

	return (
		<div className={`sidebar-draw ${appState.tools.selected.toLowerCase()}`}>
			<div className="tool-selection">
				<SvgIcon
					className="sidebar-icon"
					title={Tools.BRUSH}
					icon={iconBrush}
					onClick={() => (appState.tools.selected = Tools.BRUSH)}
					isActive={appState.tools.selected === Tools.BRUSH}
				/>
				<SvgIcon
					className="sidebar-icon"
					title={Tools.ERASER}
					icon={iconEraser}
					onClick={() => (appState.tools.selected = Tools.ERASER)}
					isActive={appState.tools.selected === Tools.ERASER}
				/>
				<SvgIcon
					className="sidebar-icon"
					title={Tools.FILL}
					icon={iconBucket}
					onClick={() => (appState.tools.selected = Tools.FILL)}
					isActive={appState.tools.selected === Tools.FILL}
				/>
				<SvgIcon
					className="sidebar-icon"
					title={Tools.GRADIENT}
					icon={iconGradient}
					onClick={() => (appState.tools.selected = Tools.GRADIENT)}
					isActive={appState.tools.selected === Tools.GRADIENT}
				/>
				<SvgIcon className="sidebar-icon" icon={iconClear} onClick={() => getCanvas().clear()} title="Clear" />
			</div>
			<div className="color-picker">{renderColorPicker()}</div>
		</div>
	);
});
