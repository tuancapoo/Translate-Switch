import type { ImageInfo } from "./image.ts";
import type { TextLine, TranslationProvider } from "./providers/types.ts";

// Thử lần lượt từng provider theo priority, lỗi hoặc timeout thì fallback sang provider tiếp theo

export interface Attempt {
  provider: string;
  ok: boolean;
  ms: number;
  error?: string;
}

export interface TranslationOutcome {
  lines: TextLine[];
  provider: string;
  attempts: Attempt[];
}

export class AllProvidersFailedError extends Error {
  readonly attempts: Attempt[];

  constructor(attempts: Attempt[]) {
    super(`all providers failed: ${attempts.map((a) => a.error).join(" | ") || "no provider configured"}`);
    this.attempts = attempts;
  }
}

export interface FallbackOptions {
  providerTimeoutMs: number;
  totalTimeoutMs: number;
}

// Không bắt đầu provider mới nếu chỉ còn ít hơn chừng này thời gian
const MIN_ATTEMPT_MS = 2_000;

export class FallbackTranslator {
  private readonly providers: TranslationProvider[];
  private readonly options: FallbackOptions;

  constructor(providers: TranslationProvider[], options: FallbackOptions) {
    this.providers = providers;
    this.options = options;
  }

  async translate(image: Uint8Array, info: ImageInfo, targetLang: string): Promise<TranslationOutcome> {
    const deadline = Date.now() + this.options.totalTimeoutMs;
    const attempts: Attempt[] = [];

    for (const provider of this.providers) {
      const remaining = deadline - Date.now();
      if (remaining < MIN_ATTEMPT_MS)
        break;

      const started = Date.now();
      try {
        const lines = await provider.translate({
          image,
          mimeType: info.mimeType,
          width: info.width,
          height: info.height,
          targetLang,
          signal: AbortSignal.timeout(Math.min(this.options.providerTimeoutMs, remaining)),
        });
        attempts.push({ provider: provider.name, ok: true, ms: Date.now() - started });
        return { lines, provider: provider.name, attempts };
      } catch (error) {
        const message = (error as Error).message;
        attempts.push({ provider: provider.name, ok: false, ms: Date.now() - started, error: message });
        console.warn(`[fallback] ${message}`);
      }
    }

    throw new AllProvidersFailedError(attempts);
  }
}
