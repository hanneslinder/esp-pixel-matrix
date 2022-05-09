import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../state/appState";
import { getContrastColor } from "../../utils/color";

import { HexColorPicker } from "react-colorful";

interface ColorPickerProps {
	onChange?: (color: string) => void;
	color?: string;
	className?: string;
}

export const ColorPicker = view((props: ColorPickerProps) => {
	return <HexColorPicker color={appState.tools.color} onChange={(color) => (appState.tools.color = color)} />;
});
