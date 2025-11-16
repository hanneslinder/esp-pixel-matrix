import clsx from "clsx";

interface NumberInputProps {
  label: string;
  value: number;
  onChange: (value: number) => void;
  min?: number;
  max?: number;
  step?: number;
}

export const NumberInput = ({
  label,
  value,
  onChange,
  min = -Infinity,
  max = Infinity,
  step = 1,
}: NumberInputProps) => {
  return (
    <>
      <div className="flex-grow items-center flex">{label}</div>
      <div className="flex-grow-0 flex items-center">
        <div>{value}</div>
        <div className="flex-grow-0 basis-10 flex ml-5 select-none">
          <button
            className={clsx(
              "btn btn-sm btn-outline bg-gray-900 border-gray-700",
              value <= min && "opacity-50 pointer-events-none"
            )}
            onClick={() => onChange(value - step)}
          >
            -
          </button>
          <button
            className={clsx(
              "btn btn-sm btn-outline bg-gray-900 border-gray-700",
              value > max && "opacity-50 pointer-events-none"
            )}
            onClick={() => onChange(value + step)}
          >
            +
          </button>
        </div>
      </div>
    </>
  );
};
