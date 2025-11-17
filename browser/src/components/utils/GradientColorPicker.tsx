import clsx from "clsx";
import React, { useMemo, useState, useRef } from "react";
import { HexColorPicker } from "react-colorful";

interface Props {
  onColor1Select: (color: string) => void;
  onColor2Select: (color: string) => void;
  color1: string;
  color2: string;
}

export const GradientColorPicker: React.FC<Props> = ({
  onColor1Select,
  onColor2Select,
  color1,
  color2,
}) => {
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

  // TODO: Multiple color stops not supported fully yet
  const handleClick = (e: React.MouseEvent<HTMLDivElement>) => {
    return;
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
    <div className="max-w-[400px] rounded-xl">
      <div className="[&_.react-colorful]:w-full">
        <HexColorPicker color={colors[index]} onChange={handleChange} />
      </div>

      <div
        className="relative flex mt-6 h-8 rounded-lg shadow-[inset_0_0_1px_#888]"
        ref={control}
        onMouseDown={handleClick}
        style={{ backgroundImage }}
      >
        {colors.map((color, i) => (
          <div
            key={i}
            onMouseDown={() => setIndex(i)}
            className={`cursor-pointer absolute top-1/2 -translate-x-1/2 -translate-y-1/2 w-2.5 rounded border border-white shadow-[0_2px_4px_rgba(0,0,0,0.2)] ${
              i === index
                ? "h-[120%] border-2 shadow-[0_2px_6px_rgba(0,0,0,0.4)]"
                : "h-[120%]"
            }`}
            style={{ backgroundColor: color, left: `${positions[i]}%` }}
          >
            <div
              className={clsx(
                "absolute top-10 w-8 text-center -left-6",
                i === index && "font-bold"
              )}
            >
              {color}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
};
