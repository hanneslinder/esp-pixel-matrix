export function convertHexTo16Bit(input: string): string {
  let RGB888 = parseInt(input.replace(/^#/, ""), 16);
  let r = (RGB888 & 0xff0000) >> 16;
  let g = (RGB888 & 0xff00) >> 8;
  let b = RGB888 & 0xff;

  r = (r * 249 + 1014) >> 11;
  g = (g * 253 + 505) >> 10;
  b = (b * 249 + 1014) >> 11;
  let RGB565 = 0;
  RGB565 = RGB565 | (r << 11);
  RGB565 = RGB565 | (g << 5);
  RGB565 = RGB565 | b;

  return "0x" + RGB565.toString(16);
}

export function rgbToHex(r: number, g: number, b: number): string {
  return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
}

export function getContrastColor(hex: string): string {
  const hexToR = (h: string) => parseInt(cutHex(h).substring(0, 2), 16);
  const hexToG = (h: string) => parseInt(cutHex(h).substring(2, 4), 16);
  const hexToB = (h: string) => parseInt(cutHex(h).substring(4, 6), 16);
  const cutHex = (h: string) => (h.charAt(0) == "#" ? h.substring(1, 7) : h);

  const threshold = 130;
  const hRed = hexToR(hex);
  const hGreen = hexToG(hex);
  const hBlue = hexToB(hex);

  const cBrightness = (hRed * 299 + hGreen * 587 + hBlue * 114) / 1000;
  return cBrightness > threshold ? "#000000" : "#ffffff";
}

export function stringToColor(input: string): string | null {
  const trimmed = input.trim();

  const hexMatch = trimmed.match(/^#?([0-9A-Fa-f]{6})$/);
  if (hexMatch) {
    return `#${hexMatch[1].toLowerCase()}`;
  }

  const rgbMatch = trimmed.match(
    /^rgb\(?\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})\s*\)?$/
  );
  if (rgbMatch) {
    const r = parseInt(rgbMatch[1]);
    const g = parseInt(rgbMatch[2]);
    const b = parseInt(rgbMatch[3]);
    if (r <= 255 && g <= 255 && b <= 255) {
      return rgbToHex(r, g, b);
    }
  }

  const simpleRgbMatch = trimmed.match(
    /^(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})$/
  );
  if (simpleRgbMatch) {
    const r = parseInt(simpleRgbMatch[1]);
    const g = parseInt(simpleRgbMatch[2]);
    const b = parseInt(simpleRgbMatch[3]);
    if (r <= 255 && g <= 255 && b <= 255) {
      return rgbToHex(r, g, b);
    }
  }

  return null;
}
