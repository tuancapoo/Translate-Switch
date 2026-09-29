// Kiểu dữ liệu dùng chung cho mọi provider (adapter)

import type { ImageMimeType } from "../image.ts";

/** Một đoạn chữ trên màn hình: vị trí theo pixel của ảnh gốc + bản gốc + bản dịch */
export interface TextLine {
  x: number;
  y: number;
  w: number;
  h: number;
  source: string;
  translation: string;
}

export interface TranslateRequest {
  image: Uint8Array;
  mimeType: ImageMimeType;
  width: number;
  height: number;
  targetLang: string;
  signal: AbortSignal;
}

/** Mọi adapter (OpenAI-compatible, Gemini, ...) đều cài interface này */
export interface TranslationProvider {
  readonly name: string;
  translate(request: TranslateRequest): Promise<TextLine[]>;
}

export class ProviderError extends Error {
  readonly provider: string;
  readonly status?: number;

  constructor(provider: string, message: string, status?: number) {
    super(`[${provider}] ${message}`);
    this.provider = provider;
    this.status = status;
  }
}
