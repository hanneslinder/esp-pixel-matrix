import React, { useRef, useState } from "react";
import { useOnClickOutside } from "../../utils/hooks";
import { ChevronDown } from "lucide-react";
import clsx from "clsx";

export interface DropdownItem {
  value: any;
  label: string;
}

interface Props {
  items: DropdownItem[];
  selected: any;
  onSelect: (selected: any) => void;
}

export const Dropdown: React.FC<Props> = ({ items, selected, onSelect }) => {
  const [open, setOpen] = useState(false);
  const ref = useRef(null);
  const selectedItem = items.find((i) => i.value === selected);

  const handleClickOutside = () => {
    setOpen(false);
  };

  const handleItemSelected = (item: DropdownItem) => {
    setOpen(false);
    onSelect(item.value);
  };

  useOnClickOutside(ref, handleClickOutside);

  return (
    <div className="relative" ref={ref}>
      <div
        className="bg-gray-900 p-2 w-32 cursor-pointer flex justify-between items-center rounded-md hover:bg-gray-700"
        onClick={() => setOpen(!open)}
      >
        <span>{selectedItem.label}</span>
        <ChevronDown />
      </div>
      {open && (
        <div className="w-full absolute top-10 right-0 z-[100] bg-gray-900">
          {items.map((item) => (
            <div
              key={item.label}
              className={clsx(
                "p-2 cursor-pointer hover:bg-gray-700",
                item.value === selected && "font-bold"
              )}
              onClick={() => handleItemSelected(item)}
            >
              {item.label}
            </div>
          ))}
        </div>
      )}
    </div>
  );
};
