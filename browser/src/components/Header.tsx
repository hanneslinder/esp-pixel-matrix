import React from "react";
import { view } from "@risingstack/react-easy-state";
import { appState } from "../state/appState";
import { VersionChecker } from "./utils/VersionChecker";

import { WifiOff, Wifi, CloudUpload, CloudDownload } from "lucide-react";

interface Props {}

export const Header: React.FC<Props> = view(() => {
	const isConnected = appState.connection.state === WebSocket.OPEN;
	const text = isConnected ? `Connected to ${(window as any).websocketUrl}` : "No connection";

	return (
		<div className="header">
			<div className="connection">
				<div className="connection-state">
					{isConnected ? <Wifi /> : <WifiOff />}
					<span>{text}</span>
				</div>
				<div className="connection-send-receive">
					{appState.connection.isSending && <CloudUpload />}
					{appState.connection.isReceiving && <CloudDownload />}
				</div>
			</div>
			<VersionChecker />
		</div>
	);
});
