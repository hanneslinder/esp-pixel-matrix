import React, { useRef, useState } from "react";
import { useOnClickOutside } from "../../utils/hooks";
import { ChevronDown } from "lucide-react";

import "./Dropdown.css";

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
		<div className={`dropdown ${open ? "is-open" : ""}`} ref={ref}>
			<div className="dropdown-selected" onClick={() => setOpen(!open)}>
				<span>{selectedItem.label}</span>
				<ChevronDown />
			</div>
			<div className="dropdown-list" style={{ height: `${items.length * 30}px` }}>
				{items.map((item) => (
					<div
						key={item.label}
						className={`dropdown-item ${item === selectedItem ? "is-selected" : ""}`}
						onClick={() => handleItemSelected(item)}
					>
						{item.label}
					</div>
				))}
			</div>
		</div>
	);
};
