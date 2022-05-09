import { PixelData } from "../components/canvas/Canvas";
import { appState, AppState } from "../state/appState";
import { createId } from "./utils";

export interface SavedItem {
	name: string;
	id: string;
	pixelData: PixelData[];
	modified: number;
	state: AppStateToPersist;
}

export type AppStateToPersist = Omit<AppState, "savedItems" | "connection" | "loadedItemId">;
export type StateFromRemote = Omit<AppState, "savedItems" | "connection" | "loadedItemId" | "matrix" | "tools" | "sidebarLeft">;
export interface PixelsFromRemote {
	action: "matrixPixels";
	data: string[][];
	layer: "bg" | "text";
	"line-start": number;
	"line-end": number;
}

export const getStoredState = (): AppStateToPersist => {
	return JSON.parse(localStorage.getItem("state"));
};

const getIndex = (): string[] => {
	return JSON.parse(localStorage.getItem("data-ids")) || [];
};

const updateIndex = (ids: string[]) => {
	localStorage.setItem("data-ids", JSON.stringify(ids));
};

const addIdToIndex = (id: string) => {
	const index = getIndex();
	const existing = index.find((i) => i === id);

	if (!existing) {
		updateIndex([...index, id]);
	}
};

const removeFromIndex = (id: string) => {
	const ids = getIndex().filter((i) => i !== id);
	updateIndex(ids);
};

export const saveView = (pixelData: PixelData[], name: string, id = createId()) => {
	console.log("Save", id, name);

	addIdToIndex(id);

	const { connection, savedItems, ...state } = appState;
	const item: SavedItem = {
		pixelData,
		id,
		name,
		state,
		modified: Date.now(),
	};

	localStorage.setItem(`data-${id}`, JSON.stringify(item));

	const existingItemIndex = appState.savedItems.findIndex((i) => i.id === id);
	appState.loadedItemId = id;

	if (existingItemIndex > -1) {
		appState.savedItems[existingItemIndex] = item;
	} else {
		appState.savedItems = [...appState.savedItems, item];
	}
};

export const deleteItem = (id: string) => {
	removeFromIndex(id);
	localStorage.removeItem(`data-${id}`);

	appState.savedItems = appState.savedItems.filter((i) => i.id !== id);
};

export const getSavedItemsFromLocalStorage = (): SavedItem[] => {
	const items: SavedItem[] = [];
	getIndex().forEach((id) => {
		const item = JSON.parse(localStorage.getItem(`data-${id}`));
		if (item) {
			items.push(item);
		}
	});

	return items;
};
