import { appState } from "../../state/appState";

export class CanvasGrid {
	private ctx: CanvasRenderingContext2D;
	private canvas: HTMLCanvasElement;

	constructor(container: HTMLElement) {
		this.canvas = document.createElement("canvas");
		this.canvas.setAttribute("id", "canvas-grid");
		this.ctx = this.canvas.getContext("2d");

		this.canvas.width = appState.matrix.width * appState.matrix.pixelRatio;
		this.canvas.height = appState.matrix.height * appState.matrix.pixelRatio;
		this.ctx.translate(0.5, 0.5);

		container.appendChild(this.canvas);

		this.drawGrid();
	}

	private drawGrid() {
		this.ctx.strokeStyle = "rgba(255, 255, 255, 0.2)";
		for (let x = 0; x <= this.canvas.width; x += appState.matrix.pixelRatio) {
			this.ctx.beginPath();
			this.ctx.moveTo(x, 0);
			this.ctx.lineTo(x, this.canvas.height);
			this.ctx.stroke();
		}
		for (let y = 0; y <= this.canvas.height; y += appState.matrix.pixelRatio) {
			this.ctx.beginPath();
			this.ctx.moveTo(0, y);
			this.ctx.lineTo(this.canvas.width + 0, y);
			this.ctx.stroke();
		}
		this.ctx.translate(0, 0);
	}
}
