export async function waitFor(ms: number): Promise<void> {
	return new Promise((resolve) => setTimeout(resolve, ms));
}

export function createId() {
	return Math.random().toString(16).slice(2);
}

export function limitNumberBetween(num: number, min: number, max: number) {
	return Math.min(Math.max(num, min), max);
}

export function uuidv4(): string {
	return (([1e7] as any) + -1e3 + -4e3 + -8e3 + -1e11).replace(/[018]/g, (c: any) =>
		(c ^ (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))).toString(16)
	);
}
