import { observe } from "@nx-js/observer-util";

import { appState, TextAlign } from "../../state/appState";
import { strftime } from "../../utils/time";
import { getWidth, renderText } from "./TextRenderer";

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
			this.updateTime();
			this.updateTimeInterval = setInterval(this.updateTime, 1000);
		});
	}

	private readonly updateTime = () => {
		this.reset();
		appState.text.forEach((text) => {
			const value = strftime(text.text, new Date());
			const width = getWidth(value, text.size);

			let xVal = 0;

			if (text.align === TextAlign.CENTER) {
				xVal = (appState.matrix.width - width) / 2;
			} else if (text.align === TextAlign.RIGHT) {
				xVal = appState.matrix.width - width;
			}

			const yVal = text.line === 1 ? 5 : 23;

			renderText(this.ctx, value, text.color, xVal + text.offsetX, yVal + text.offsetY, text.size);
		});
	};

	private reset(): void {
		this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
	}
}
