import React from "react";
import { view } from "@risingstack/react-easy-state";
import { appState } from "../state/appState";
import { SvgIcon } from "./utils/SvgIcon";
import { VersionChecker } from "./utils/VersionChecker";

const iconConnected = require("../assets/reception-4.svg");
const iconDisconnected = require("../assets/wifi-off.svg");
const iconSending = require("../assets/cloud-arrow-up.svg");
const iconReceiving = require("../assets/cloud-arrow-down.svg");

interface Props {}

export const Header: React.FC<Props> = view(() => {
	const isConnected = appState.connection.state === WebSocket.OPEN;
	const text = isConnected ? `Connected to ${(window as any).websocketUrl}` : "No connection";

	return (
		<div className="header">
			<div className="connection">
				<div className="connection-state">
					<SvgIcon icon={isConnected ? iconConnected : iconDisconnected} title={text} />
					<span>{text}</span>
				</div>
				<div className="connection-send-receive">
					{appState.connection.isSending && <SvgIcon icon={iconSending} title="sending data" />}
					{appState.connection.isReceiving && <SvgIcon icon={iconReceiving} title="receiving data" />}
				</div>
			</div>
			<VersionChecker />
		</div>
	);
});
