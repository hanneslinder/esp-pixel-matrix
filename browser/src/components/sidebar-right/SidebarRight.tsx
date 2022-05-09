import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../state/appState";
import { Canvas } from "../canvas/Canvas";
import { SidebarLeftItem } from "../sidebar-left/SidebarLeft";
import { SidebarBackground } from "./background/SidebarBackground";
import { SidebarText } from "./text/SidebarText";
import { SidebarSettings } from "./settings/SidebarSettings";
import { SidebarDraw } from "./draw/SidebarDraw";

import "./SidebarRight.less";

export const enum Tools {
	BRUSH = "Brush",
	ERASER = "Eraser",
	FILL = "Fill",
	CLEAR = "Clear",
	GRADIENT = "Gradient",
}

interface Props {
	getCanvas: () => Canvas;
}

export const SidebarRight: React.FC<Props> = view(({ getCanvas }) => {
	const renderActiveView = (selected: SidebarLeftItem) => {
		switch (selected) {
			case SidebarLeftItem.Draw:
				return <SidebarDraw getCanvas={getCanvas} />;
			case SidebarLeftItem.Text:
				return <SidebarText />;
			case SidebarLeftItem.Background:
				return <SidebarBackground getCanvas={getCanvas} />;
			case SidebarLeftItem.Settings:
				return <SidebarSettings getCanvas={getCanvas} />;
		}
	};

	return <div className="sidebar-right-content">{renderActiveView(appState.sidebarLeft.selected)}</div>;
});
