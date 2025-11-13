import React from "react";
import { SvgIcon } from "./SvgIcon";

interface IconButtonProps extends React.HTMLProps<HTMLDivElement> {
  icon: string;
  text: string;
  iconPosition?: "left" | "right";
}

export const IconButton = (props: IconButtonProps) => {
  const { icon, text, iconPosition, ...divProps } = props;
  const svgIcon = <SvgIcon icon={props.icon} />;
  let iconLeft: JSX.Element, iconRight: JSX.Element;

  let className = "inline-block rounded-md cursor-pointer";
  if (props.className) {
    className += ` ${props.className}`;
  }

  if (!props.iconPosition || props.iconPosition === "left") {
    iconLeft = svgIcon;
  } else {
    iconRight = svgIcon;
  }

  return (
    <div {...divProps} className={className}>
      {iconLeft}
      <span
        className={`text-[--color-highlight-2] ${iconLeft ? "pl-2.5" : ""} ${
          iconRight ? "pr-2.5" : ""
        }`}
      >
        {props.text}
      </span>
      {iconRight}
    </div>
  );
};
