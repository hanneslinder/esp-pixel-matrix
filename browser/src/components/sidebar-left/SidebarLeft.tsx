import { view } from "@risingstack/react-easy-state";
import { appState } from "../../state/appState";
import { Brush, Type, Image, Settings } from "lucide-react";

import "./SidebarLeft.css";

export const enum SidebarLeftItem {
  Draw = "Draw",
  Text = "Text",
  Background = "Background",
  Settings = "Settings",
}

interface Props {}

export const SidebarLeft = view((props: Props) => {
  const items = [
    { label: SidebarLeftItem.Draw, icon: <Brush /> },
    { label: SidebarLeftItem.Text, icon: <Type /> },
    { label: SidebarLeftItem.Background, icon: <Image /> },
    { label: SidebarLeftItem.Settings, icon: <Settings /> },
  ];

  const setItemActive = (item: SidebarLeftItem) =>
    (appState.sidebarLeft.selected = item);

  return (
    <div className="sidebar-left-content">
      {items.map((item) => {
        const className = `sidebar-icon ${
          appState.sidebarLeft.selected === item.label ? "is-active" : ""
        }`;

        return (
          <div
            key={item.label}
            className={className}
            onClick={() => setItemActive(item.label)}
          >
            <div>{item.icon}</div>
            <div>{item.label}</div>
          </div>
        );
      })}
    </div>
  );
});
