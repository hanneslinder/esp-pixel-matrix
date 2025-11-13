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
            className={`cursor-pointer flex flex-col items-center justify-center h-20 w-10 px-5 text-xs border-t border-[--color-dark-3] last:border-b ${
              isActive
                ? "bg-[--color-highlight-1]"
                : "hover:bg-[--color-dark-3]"
            }`}
            onClick={() => setItemActive(item.label)}
          >
            <div className="mb-1.5 [&>svg]:stroke-[1px]">{item.icon}</div>
            <div>{item.label}</div>
          </div>
        );
      })}
    </div>
  );
});
