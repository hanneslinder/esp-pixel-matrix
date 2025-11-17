import { view } from "@risingstack/react-easy-state";
import { appState } from "../state/appState";
import { Brush, Type, Image, Settings } from "lucide-react";

export const enum Views {
  Draw = "Draw",
  Text = "Text",
  Background = "Background",
  Settings = "Settings",
}

interface Props {}

export const Navigation = view((props: Props) => {
  const items = [
    { label: Views.Draw, icon: <Brush /> },
    { label: Views.Text, icon: <Type /> },
    { label: Views.Background, icon: <Image /> },
    { label: Views.Settings, icon: <Settings /> },
  ];

  const setItemActive = (item: Views) => (appState.view = item);

  return (
    <div className="flex flex-col select-none">
      {items.map((item) => {
        const isActive = appState.view === item.label;

        return (
          <div
            key={item.label}
            className={`cursor-pointer flex flex-col items-center justify-center h-20 px-2 text-xs border-gray-700 ${
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
