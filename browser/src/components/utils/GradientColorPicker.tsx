import React, { useMemo, useState, useRef } from "react";
import { HexColorPicker } from "react-colorful";
import "./GradientColorPicker.less";

interface Props {
	onColor1Select: (color: string) => void;
	onColor2Select: (color: string) => void;
	color1: string;
	color2: string;
}

export const GradientColorPicker: React.FC<Props> = ({ onColor1Select, onColor2Select, color1, color2 }) => {
	const control = useRef<HTMLDivElement>();
	const [positions, setPositions] = useState([0, 100]);
	const [colors, setColors] = useState([color1, color2]);
	const [index, setIndex] = useState(0);

	const backgroundImage = useMemo(() => {
		const stopStrings = colors.map((color, i) => {
			return `${color} ${positions[i]}%`;
		});

		return `linear-gradient(to right, ${stopStrings.join(", ")})`;
	}, [colors, positions]);

	const handleChange = (hex: string) => {
		const colorsCopy = colors.slice();
		colorsCopy[index] = hex;
		setColors(colorsCopy);

		if (index === 0) {
			onColor1Select(hex);
		} else {
			onColor2Select(hex);
		}
	};

	const handleClick = (e: React.MouseEvent<HTMLDivElement>) => {
		if (e.target === control.current) {
			const { x, width } = control.current.getBoundingClientRect();
			const position = ((e.clientX - x) / width) * 100;
			const afterIndex = positions.findIndex((p) => p > position);

			const colorsCopy = colors.slice();
			const positionsCopy = positions.slice();

			colorsCopy.splice(afterIndex, 0, "#000000");
			positionsCopy.splice(afterIndex, 0, position);

			setColors(colorsCopy);
			setPositions(positionsCopy);
			setIndex(afterIndex);
		}
	};

	return (
		<div className="gradient-color-picker">
			<div className="control" ref={control} onMouseDown={handleClick} style={{ backgroundImage }}>
				{colors.map((color, i) => (
					<div
						key={i}
						onMouseDown={() => setIndex(i)}
						className={`stop ${i === index ? "active" : ""}`}
						style={{ backgroundColor: color, left: `${positions[i]}%` }}
					></div>
				))}
			</div>

			<HexColorPicker color={colors[index]} onChange={handleChange} />
		</div>
	);
};
