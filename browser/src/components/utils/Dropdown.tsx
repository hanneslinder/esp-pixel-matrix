import React, { useRef, useState } from "react";
import { useOnClickOutside } from "../../utils/hooks";
import { ChevronDown } from "lucide-react";

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
    <div className="relative w-[100px]" ref={ref}>
      <div
        className="bg-[--color-dark-1] px-2.5 py-1.5 w-[100px] box-border cursor-pointer flex justify-between rounded-md hover:bg-[--color-blue-0] [&>svg]:w-3"
        onClick={() => setOpen(!open)}
      >
        <span>{selectedItem.label}</span>
        <ChevronDown />
      </div>
      {open && (
        <div
          className="w-[100px] absolute top-[31px] right-0 z-[100] bg-[--color-dark-1]"
          style={{ height: `${items.length * 30}px` }}
        >
          {items.map((item) => (
            <div
              key={item.label}
              className="px-2.5 py-1.5 cursor-pointer hover:bg-[--color-blue-0]"
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
