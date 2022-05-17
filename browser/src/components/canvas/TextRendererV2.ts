// Based on https://github.com/tchapi/Adafruit-GFX-Font-Customiser

import { font } from "../../assets/font";
import { bitmaps, glyphs, metaData } from "../../assets/fonts/pico";
import { appState } from "../../state/appState";

const fontWidth = 5;
const fontHeight = 6;

const fontInfo = {
	firstChar: 0,
	lastChar: 0,
	maxHeight: 0,
	maxWidth: 0,
	baseLine: 0,
	underBaseLine: 0,
};

export const getWidth = (text: string, textSize = 1) => {
	// Character width + space width
	return text.length * fontWidth * textSize + (text.length - 1) * textSize;
};

export const renderText = (ctx: CanvasRenderingContext2D, text: string, color: string, posX: number, posY: number, textSize = 1) => {
	initFont();

	ctx.fillStyle = color;

	let offsetX = posX;
	let offsetY = posY;
	text.split("").forEach((t) => {
		const letterWidth = renderLetter(ctx, t, offsetX, offsetY);
		offsetX += letterWidth;
	});
};

function renderLetter(ctx: CanvasRenderingContext2D, letter: string, offsetX: number, offsetY: number) {
	let nextOffsetX = 0;

	glyphs.forEach((g, i) => {
		// find desired character in glyphs array
		const char = String.fromCharCode(fontInfo.firstChar + i);

		if (char === letter) {
			const [idx, w, h, adv, ow, oh] = g;
			let pixels = "";
			let nextOffset = 0;

			if (i + 1 < glyphs.length) {
				let nextIndexInc = 1;
				do {
					nextOffset = glyphs[i + nextIndexInc][0];
					nextIndexInc += 1;
				} while (nextOffset === 0 && i + nextIndexInc < glyphs.length);

				if (nextOffset === 0) {
					nextOffset = bitmaps.length;
				}
			} else {
				nextOffset = bitmaps.length;
			}

			let disabled = (w == 0 || h == 0) && adv == 0;
			if (!disabled) {
				for (let k = 0; k < nextOffset - idx; k++) {
					pixels += ("000000000" + bitmaps[idx + k].toString(2)).substr(-8);
				}
			}

			nextOffsetX = w + 1;
			drawGlyphPixels(ctx, pixels, w, h, char, adv, ow, oh, offsetX, offsetY, disabled);
		}
	});

	return nextOffsetX;
}

function drawGlyphPixels(
	ctx: CanvasRenderingContext2D,
	pixels: string,
	w: number,
	h: number,
	char: string,
	adv: number,
	ow: number,
	oh: number,
	offsetX: number,
	offsetY: number,
	disabled: boolean
) {
	const ratio = appState.matrix.pixelRatio;
	const left = ow;
	const right = w + ow;
	const top = fontInfo.baseLine + oh;
	const bottom = top + h;

	for (let y = 0; y < fontInfo.maxHeight; y++) {
		for (let x = Math.min(0, ow); x <= Math.max(adv, right); x++) {
			if (y < top || y >= bottom || x < left || x >= right) {
				// do nothing
			} else if (pixels.charAt((y - top) * w + (x - left)) === "1") {
				ctx.fillRect(offsetX * ratio + x * ratio, offsetY * ratio + y * ratio, ratio - 1, ratio - 1);
			}
		}
	}
}

// extract max properties from given font
function initFont() {
	fontInfo.firstChar = metaData[0];
	fontInfo.lastChar = metaData[1];

	glyphs.forEach((g) => {
		const inv_dy = -g[5];
		fontInfo.maxWidth = Math.max(fontInfo.maxWidth, g[1], g[3]);
		fontInfo.baseLine = Math.max(fontInfo.baseLine, inv_dy);
		fontInfo.underBaseLine = Math.min(fontInfo.underBaseLine, inv_dy + 1 - g[2]);
	});

	fontInfo.maxHeight = fontInfo.baseLine + 1 - fontInfo.underBaseLine;
}
