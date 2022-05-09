import * as React from "react";

import "./SvgIcon.less";

export interface ISvgIconProps extends React.HTMLProps<HTMLSpanElement> {
	icon: string;
	color?: string;
	disabled?: boolean;
	isActive?: boolean;
}

export const SvgIcon = (props: ISvgIconProps) => {
	let __html = props.icon,
		iconStyle: React.CSSProperties = {},
		className = "svg-icon " + (props.className ? props.className : "");

	if (props.onClick && props.disabled !== true) {
		iconStyle.cursor = "pointer";
	}

	if (props.disabled) {
		className += " is-disabled";
	}

	if (props.style) {
		iconStyle = { ...iconStyle, ...props.style };
	}

	if (props.isActive) {
		className += " is-active";
	}

	if (props.color) {
		iconStyle.fill = props.color;
		className += " inherit-color";
	}

	const { isActive, icon, color, ...propsToSpread } = props;

	return <span {...propsToSpread} style={iconStyle} className={className} dangerouslySetInnerHTML={{ __html }} />;
};
