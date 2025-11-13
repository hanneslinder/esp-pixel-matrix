import React from "react";
import { view } from "@risingstack/react-easy-state";
import { appState } from "../state/appState";
import { VersionChecker } from "./utils/VersionChecker";

import { WifiOff, Wifi, CloudUpload, CloudDownload } from "lucide-react";

interface Props {}

export const Header: React.FC<Props> = view(() => {
  const isConnected = appState.connection.state === WebSocket.OPEN;
  const text = isConnected
    ? `Connected to ${(window as any).websocketUrl}`
    : "No connection";

  return (
    <div className="flex-grow-0 basis-10 flex bg-[--color-dark-2] items-center justify-center">
      <div className="flex text-[--color-text-2] text-sm">
        <div className="flex mr-5">
          {isConnected ? (
            <Wifi className="h-3 translate-y-[3px]" />
          ) : (
            <WifiOff className="h-3 translate-y-[3px]" />
          )}
          <span>{text}</span>
        </div>
        <div className="ml-5">
          {appState.connection.isSending && (
            <CloudUpload className="h-3 translate-y-[3px]" />
          )}
          {appState.connection.isReceiving && (
            <CloudDownload className="h-3 translate-y-[3px]" />
          )}
        </div>
      </div>
      <VersionChecker />
    </div>
  );
});
