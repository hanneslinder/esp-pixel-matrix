import { font } from "../../assets/font";
import { appState } from "../../state/appState";

const fontWidth = 5;
const fontHeight = 8;

export const getWidth = (text: string, textSize = 1) => {
  // Character width + space width
  return text.length * fontWidth * textSize + (text.length - 1) * textSize;
};

export const renderText = (
  ctx: CanvasRenderingContext2D,
  text: string,
  color: string,
  posX: number,
  posY: number,
  textSize = 1
) => {
  const letters = [];

  ctx.fillStyle = color;

  for (let l = 0; l < text.length; l++) {
    const charCode = text.charCodeAt(l);
    const t = font.filter((_e, i, arr) => {
      if (i >= charCode * 5 && i <= charCode * 5 + 4) {
        return arr[i];
      }
    });

    letters.push(t);

    // keep track of empty columns so that we can shift charactes into the center if required
    let numEmptyCols = 0;
    let firstColToDraw;

    // iterate over each pixel in the 8x5 font rectangle and check if we need to draw something
    // in order to correctly position characters that are less than 5px wide we need to start
    // drawing each letter from right to left and offset if required
    for (let x = fontWidth; x >= 0; x--) {
      let emptyPixelInColumn = 0;

      for (let y = 0; y < fontHeight; y++) {
        const letterPixel = (letters[l][x] >> y) & 1;

        // Draw a single pixel
        if (letterPixel) {
          let offsetX = 0;

          // track which column is the first that has data
          if (firstColToDraw === undefined) {
            firstColToDraw = x;
          }

          // add offset if the character would be off centered
          if (firstColToDraw <= 2) {
            offsetX = Math.floor(numEmptyCols / 2);
          }

          for (let pixelX = 0; pixelX < textSize; pixelX++) {
            for (let pixelY = 0; pixelY < textSize; pixelY++) {
              const px =
                posX * appState.settings.pixelRatio +
                (x + offsetX) * textSize * appState.settings.pixelRatio +
                l * 6 * textSize * appState.settings.pixelRatio +
                1 +
                appState.settings.pixelRatio * pixelX;
              const py =
                posY * appState.settings.pixelRatio +
                y * textSize * appState.settings.pixelRatio +
                1 +
                appState.settings.pixelRatio * pixelY;
              ctx.fillRect(
                px,
                py,
                appState.settings.pixelRatio - 1,
                appState.settings.pixelRatio - 1
              );
            }
          }
        } else {
          // no pixel was drawn for that y position
          emptyPixelInColumn++;

          // no pixel was drawn for the whole column
          if (emptyPixelInColumn === fontHeight) {
            numEmptyCols++;
          }
        }
      }
    }
  }

  ctx.translate(0, 0);
};
