import { view } from "@risingstack/react-easy-state";
import { appState } from "../../state/appState";
import { Brush, Type, Image, Settings } from "lucide-react";

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
    <div className="flex flex-col select-none">
      {items.map((item) => {
        const isActive = appState.sidebarLeft.selected === item.label;

        return (
          <div
            key={item.label}
            className={`cursor-pointer flex flex-col items-center justify-center h-20 px-2 text-xs border-t border-gray-600 last:border-b ${
              isActive ? "bg-gray-900" : "hover:bg-gray-700"
            }`}
            onClick={() => setItemActive(item.label)}
          >
            <div className="mb-1 mb-2">{item.icon}</div>
            <div className="text-xs">{item.label}</div>
          </div>
        );
      })}
    </div>
  );
});
