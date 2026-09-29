import type { TextLine } from "./providers/types.ts";

/**
 * Cách model trả toạ độ:
 * - "pixels": x, y, w, h theo pixel của ảnh (đa số model)
 * - "box_2d": [ymin, xmin, ymax, xmax] chuẩn hoá 0-1000 (Gemini được train theo kiểu này, chính xác hơn)
 */
export type CoordinateMode = "pixels" | "box_2d";

export function buildPrompt(targetLang: string, width: number, height: number, mode: CoordinateMode): string {
  const boxFormat =
    mode === "pixels"
      ? `"x", "y", "w", "h": integer bounding box of the text in pixels of the ${width}x${height} image`
      : `"box_2d": [ymin, xmin, ymax, xmax] of the text, normalized to 0-1000`;

  return `This is a ${width}x${height} screenshot from a Nintendo Switch game.
Find every piece of readable text (dialogue, menus, item names, UI labels) and translate it into ${targetLang}.

Rules:
- Group text that belongs together (one dialogue box, one menu entry) into a single entry.
- Keep character names and button glyphs such as (A) or (B) unchanged.
- Use a natural, game-appropriate ${targetLang} style.
- Skip logos and purely decorative text. If there is no text, return an empty list.

Reply with JSON only, no markdown, in this shape:
{"lines": [{ ${boxFormat}, "source": "original text", "translation": "translated text" }]}`;
}

/** Lấy object JSON trong câu trả lời, kể cả khi model bọc trong ```json ... ``` */
export function extractJson(text: string): unknown {
  const start = text.indexOf("{");
  const end = text.lastIndexOf("}");
  if (start === -1 || end <= start)
    throw new Error("no JSON object in model output");
  return JSON.parse(text.slice(start, end + 1));
}

const clamp = (value: number, min: number, max: number) => Math.min(max, Math.max(min, Math.round(value)));

/** Kiểm tra và chuẩn hoá output của model về TextLine (toạ độ pixel) */
export function normalizeLines(data: unknown, width: number, height: number, mode: CoordinateMode): TextLine[] {
  const lines = (data as { lines?: unknown })?.lines;
  if (!Array.isArray(lines))
    throw new Error('model output has no "lines" array');

  return lines.flatMap((raw): TextLine[] => {
    const item = raw as Record<string, unknown>;
    if (typeof item.translation !== "string" || item.translation.trim() === "")
      return [];

    let x: number, y: number, w: number, h: number;
    if (mode === "box_2d") {
      const box = item.box_2d;
      if (!Array.isArray(box) || box.length !== 4 || !box.every((v) => typeof v === "number"))
        return [];
      const [ymin, xmin, ymax, xmax] = box as number[];
      x = (xmin / 1000) * width;
      y = (ymin / 1000) * height;
      w = ((xmax - xmin) / 1000) * width;
      h = ((ymax - ymin) / 1000) * height;
    } else {
      [x, y, w, h] = [item.x, item.y, item.w, item.h].map(Number);
      if (![x, y, w, h].every(Number.isFinite))
        return [];
    }

    const left = clamp(x, 0, width);
    const top = clamp(y, 0, height);
    return [{
      x: left,
      y: top,
      w: clamp(w, 1, width - left),
      h: clamp(h, 1, height - top),
      source: typeof item.source === "string" ? item.source : "",
      translation: item.translation,
    }];
  });
}
