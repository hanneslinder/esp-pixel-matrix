import { observe } from "@nx-js/observer-util";

import { appState, Font, TextAlign } from "../../state/appState";
import { strftime } from "../../utils/time";
import * as DefaultTextRenderer from "./TextRenderer";
import * as V2TextRenderer from "./TextRendererV2";

export class CanvasTextLayer {
	private ctx: CanvasRenderingContext2D;
	private canvas: HTMLCanvasElement;
	private updateTimeInterval: any;

	constructor(container: HTMLElement) {
		this.canvas = document.createElement("canvas");
		this.canvas.setAttribute("id", "canvas-text-layer");
		this.ctx = this.canvas.getContext("2d");

		this.canvas.width = appState.matrix.width * appState.matrix.pixelRatio;
		this.canvas.height = appState.matrix.height * appState.matrix.pixelRatio;
		this.ctx.translate(0.5, 0.5);

		container.appendChild(this.canvas);

		this.observeChanges();
	}

	private observeChanges() {
		observe(() => {
			this.reset();
			clearInterval(this.updateTimeInterval);
			this.updateText();
			// this.updateTimeInterval = setInterval(this.updateTime, 1000);
		});
	}

	private readonly updateText = () => {
		this.reset();

		appState.text.forEach((text) => {
			const renderFunction = text.font === Font.REGULAR ? DefaultTextRenderer.renderText : V2TextRenderer.renderText;
			const widthFunction = text.font === Font.REGULAR ? DefaultTextRenderer.getWidth : V2TextRenderer.getWidth;
			
			const value = strftime(text.text, new Date());
			const width = widthFunction(value, text.size);
			const yVal = text.line === 1 ? 5 : 23;

			let xVal = 0;
			if (text.align === TextAlign.CENTER) {
				xVal = (appState.matrix.width - width) / 2;
			} else if (text.align === TextAlign.RIGHT) {
				xVal = appState.matrix.width - width;
			}

			renderFunction(this.ctx, value, text.color, Math.round(xVal + text.offsetX), Math.round(yVal + text.offsetY), text.size);
		});
	};

	private reset(): void {
		this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
	}
}
