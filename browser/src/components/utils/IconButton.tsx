import React from "react";
import { SvgIcon } from "./SvgIcon";

import "./IconButton.css";

interface IconButtonProps extends React.HTMLProps<HTMLDivElement> {
	icon: string;
	text: string;
	iconPosition?: "left" | "right";
}

export const IconButton = (props: IconButtonProps) => {
	const { icon, text, iconPosition, ...divProps } = props;
	const svgIcon = <SvgIcon icon={props.icon} />;
	let iconLeft: JSX.Element, iconRight: JSX.Element;

	let className = "icon-button";
	if (props.className) {
		className += ` ${props.className}`;
	}

	if (!props.iconPosition || props.iconPosition === "left") {
		iconLeft = svgIcon;
		className += " icon-left";
	} else {
		iconRight = svgIcon;
		className += " icon-right";
	}

	return (
		<div {...divProps} className={className}>
			{iconLeft}
			<span className="icon-button-text">{props.text}</span>
			{iconRight}
		</div>
	);
};
