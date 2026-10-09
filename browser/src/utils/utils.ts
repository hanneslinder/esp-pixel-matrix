export async function waitFor(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

type Debounced<Args extends any[]> = ((...args: Args) => void) & {
  flush: () => void;
  cancel: () => void;
};

export function debounce<Args extends any[]>(
  fn: (...args: Args) => void,
  waitMs: number,
): Debounced<Args> {
  let timer: ReturnType<typeof setTimeout> | undefined;
  let pendingArgs: Args | undefined;

  const invoke = () => {
    if (timer !== undefined) {
      clearTimeout(timer);
      timer = undefined;
    }

    if (pendingArgs === undefined) {
      return;
    }

    const args = pendingArgs;
    pendingArgs = undefined;
    fn(...args);
  };

  const debounced = ((...args: Args) => {
    pendingArgs = args;

    if (timer !== undefined) {
      clearTimeout(timer);
    }

    timer = setTimeout(invoke, waitMs);
  }) as Debounced<Args>;

  debounced.flush = invoke;
  debounced.cancel = () => {
    if (timer !== undefined) {
      clearTimeout(timer);
      timer = undefined;
    }

    pendingArgs = undefined;
  };

  return debounced;
}

export function createId() {
  return Math.random().toString(16).slice(2);
}

export function clampValue(num: number, min: number, max: number) {
  return Math.min(Math.max(num, min), max);
}

export function uuidv4(): string {
  return (([1e7] as any) + -1e3 + -4e3 + -8e3 + -1e11).replace(
    /[018]/g,
    (c: any) =>
      (
        c ^
        (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))
      ).toString(16),
  );
}
