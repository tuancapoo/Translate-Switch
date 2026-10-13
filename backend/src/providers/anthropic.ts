import Anthropic from "@anthropic-ai/sdk";
import { buildPrompt, extractJson, normalizeLines } from "../prompt.ts";
import { ProviderError, type TextLine, type TranslateRequest, type TranslationProvider } from "./types.ts";

// Adapter cho Claude (Anthropic Messages API) qua SDK chính thức

export interface AnthropicOptions {
  name: string;
  apiKey: string;
  model: string;
}

// Các dòng Claude 5 hỗ trợ output_config.effort. Model cũ hơn (vd Haiku 4.5) báo lỗi nếu nhận tham số này.
const SUPPORTS_EFFORT = /^claude-(opus|sonnet|fable|mythos)-5/;

export class AnthropicProvider implements TranslationProvider {
  readonly name: string;
  private readonly client: Anthropic;
  private readonly model: string;

  constructor(options: AnthropicOptions) {
    this.name = options.name;
    this.model = options.model;
    // maxRetries = 0: việc thử lại do FallbackTranslator đảm nhiệm, tránh SDK tự retry làm tốn thời gian
    this.client = new Anthropic({ apiKey: options.apiKey, maxRetries: 0 });
  }

  async translate(request: TranslateRequest): Promise<TextLine[]> {
    const { width, height } = request;

    let response: Anthropic.Message;
    try {
      response = await this.client.messages.create(
        {
          model: this.model,
          max_tokens: 4000,
          // Dịch ảnh chụp là tác vụ đơn giản cần nhanh: effort thấp
          ...(SUPPORTS_EFFORT.test(this.model) ? { output_config: { effort: "low" as const } } : {}),
          messages: [
            {
              role: "user",
              content: [
                {
                  type: "image",
                  source: {
                    type: "base64",
                    media_type: request.mimeType,
                    data: Buffer.from(request.image).toString("base64"),
                  },
                },
                { type: "text", text: buildPrompt(request.targetLang, width, height, "pixels") },
              ],
            },
          ],
        },
        { signal: request.signal },
      );
    } catch (error) {
      throw this.toProviderError(error, request.signal);
    }

    if (response.stop_reason === "refusal")
      throw new ProviderError(this.name, `refused (${response.stop_details?.category ?? "no category"})`);

    const text = response.content
      .filter((block): block is Anthropic.TextBlock => block.type === "text")
      .map((block) => block.text)
      .join("");
    if (!text)
      throw new ProviderError(this.name, `empty response (${response.stop_reason ?? "no stop reason"})`);

    try {
      return normalizeLines(extractJson(text), width, height, "pixels");
    } catch (error) {
      throw new ProviderError(this.name, `invalid output: ${(error as Error).message}`);
    }
  }

  private toProviderError(error: unknown, signal: AbortSignal): ProviderError {
    if (signal.aborted)
      return new ProviderError(this.name, "request failed: timeout");
    if (error instanceof Anthropic.APIError) {
      // Lấy message gọn trong thân lỗi ({"error":{"message":...}}) thay vì cả chuỗi nhiều dòng của SDK
      const body = error.error as { error?: { message?: unknown } } | undefined;
      const detail = typeof body?.error?.message === "string" ? body.error.message : error.message;
      return new ProviderError(this.name, `HTTP ${error.status ?? "?"}: ${detail}`.slice(0, 400), error.status);
    }
    return new ProviderError(this.name, `request failed: ${(error as Error).message}`);
  }
}
