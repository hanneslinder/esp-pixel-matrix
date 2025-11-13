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
    <div className="flex-grow-0 basis-10 flex bg-gray-800 items-center justify-center">
      <div className="flex text-xs">
        <div className="flex items-center mr-5">
          {isConnected ? <Wifi size={12} /> : <WifiOff size={12} />}
          <span className="ml-2">{text}</span>
        </div>
        <div className="ml-5">
          {appState.connection.isSending && <CloudUpload size={12} />}
          {appState.connection.isReceiving && <CloudDownload size={12} />}
        </div>
      </div>
      <VersionChecker />
    </div>
  );
});
