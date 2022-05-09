import { view } from "@risingstack/react-easy-state";
import React from "react";
import { setLocaleAction } from "../../../Actions";
import { appState } from "../../../state/appState";
import { Dropdown, DropdownItem } from "../../utils/DropDown";

interface Props {}

const localeOptions: DropdownItem[] = [
	{ label: "en_US", value: "en_US.UTF-8" },
	{ label: "de_DE", value: "de_DE.UTF-8" },
];

export const Locale: React.FC<Props> = view(() => {
	const onLocaleChanged = (selectedLocale: string) => {
		setLocaleAction(selectedLocale);
	};

	return (
		<div className="settings-item locale">
			<div className="settings-item-label">Locale</div>
			<div className="settings-item-value">
				<Dropdown selected={appState.settings.locale} items={localeOptions} onSelect={onLocaleChanged} />
			</div>
		</div>
	);
});
