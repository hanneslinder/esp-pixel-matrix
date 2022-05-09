import { PixelData } from "./components/canvas/Canvas";
import { appState, CustomDataOptions } from "./state/appState";
import { convertHexTo16Bit } from "./utils/color";
import { waitFor } from "./utils/utils";
import { getSocket } from "./Websocket";

const socket = getSocket();

export const clearCanvasAction = () => {
	const msg = {
		action: "clear",
	};

	socket.send(msg);
};

export const setTextAction = () => {
	const textToSend = appState.text.map((t) => ({
		...t,
		color: convertHexTo16Bit(t.color),
	}));

	const msg = {
		action: "setText",
		text: textToSend,
	};

	socket.send(msg);
};

// Not really an action at the moment. Might be a todo for later...
// For now just send state as separate actions and add a bit of delay in between
export const syncFullStateAction = async (pixelData: PixelData[]) => {
	setTextAction();
	await waitFor(100);

	setCustomDataAction(appState.customData);
	await waitFor(100);

	setCompositionModeAction(appState.settings.compositionMode);
	await waitFor(100);

	setBrightnessAction(appState.settings.brightness);
	await waitFor(100);

	drawImageAction(pixelData);
};

export const setCustomDataAction = (options: CustomDataOptions) => {
	appState.customData = options;

	const msg = {
		action: "customData",
		options,
	};

	socket.send(msg);
};

export const drawPixelAction = async (pixelData: PixelData[], chunkSize = 25): Promise<void> => {
	appState.connection.isSending = true;

	for (let i = 0; i < pixelData.length; i += chunkSize) {
		sendPixels(pixelData.slice(i, i + chunkSize));
		await waitFor(50);
	}

	appState.connection.isSending = false;

	return Promise.resolve();
};

export const drawImageAction = (pixelData: PixelData[]) => {
	const mappedPixelData = pixelData.map((d) => convertHexTo16Bit(d.c));
	const smallImage = mappedPixelData.filter((d, i) => i < 64 * 200);

	const msg = {
		action: "drawImage",
		data: smallImage,
	};

	socket.send(msg);
};

const sendPixels = (pixelData: PixelData[]) => {
	const mappedPixelData = pixelData.map((d) => ({ p: d.p, c: convertHexTo16Bit(d.c) }));

	const msg = {
		action: "drawpixel",
		data: mappedPixelData,
	};

	socket.send(msg);
};

export const getPixelsAction = () => {
	const msg = {
		action: "getPixels",
	};

	socket.send(msg);
};

export const getStateAction = () => {
	const msg = {
		action: "getState",
	};

	socket.send(msg);
};

export const fillAction = (color: string) => {
	const msg = {
		action: "fill",
		color: convertHexTo16Bit(color),
	};

	socket.send(msg);
};

export const setCompositionModeAction = (mode: number) => {
	if (mode > 2) {
		mode = 0;
	}

	const msg = {
		action: "compositionMode",
		mode,
	};

	appState.settings.compositionMode = mode;
	socket.send(msg);
};

export const setBrightnessAction = (brightness: number) => {
	const msg = {
		action: "setBrightness",
		brightness,
	};

	appState.settings.brightness = brightness;
	socket.send(msg);
};

export const setLocaleAction = (locale: string) => {
	const msg = {
		action: "setLocale",
		locale,
	};

	appState.settings.locale = locale;
	socket.send(msg);
};

export const resetAction = () => {
	const msg = {
		action: "reset",
	};

	socket.send(msg);
};
