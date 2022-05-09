import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../state/appState";
import { SvgIcon } from "../utils/SvgIcon";

const iconBrush = require("../../assets/brush.svg");
const iconBackground = require("../../assets/card-image.svg");
const iconText = require("../../assets/fonts.svg");
const iconSettings = require("../../assets/sliders.svg");

import "./SidebarLeft.less";

export const enum SidebarLeftItem {
	Draw = "Draw",
	Text = "Text",
	Background = "Background",
	Settings = "Settings",
}

interface Props {}

export const SidebarLeft = view((props: Props) => {
	const items = [
		{ label: SidebarLeftItem.Draw, icon: iconBrush },
		{ label: SidebarLeftItem.Text, icon: iconText },
		{ label: SidebarLeftItem.Background, icon: iconBackground },
		{ label: SidebarLeftItem.Settings, icon: iconSettings },
	];

	const setItemActive = (item: SidebarLeftItem) => (appState.sidebarLeft.selected = item);

	return (
		<div className="sidebar-left-content">
			{items.map((item) => {
				const className = `sidebar-icon ${appState.sidebarLeft.selected === item.label ? "is-active" : ""}`;

				return (
					<div key={item.label} className={className} onClick={() => setItemActive(item.label)}>
						<SvgIcon icon={item.icon} />
						<div>{item.label}</div>
					</div>
				);
			})}
		</div>
	);
});
