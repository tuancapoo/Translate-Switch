import { buildPrompt, extractJson, normalizeLines } from "../prompt.ts";
import { postJson } from "./http.ts";
import { ProviderError, type TextLine, type TranslateRequest, type TranslationProvider } from "./types.ts";

// Adapter cho mọi API chuẩn OpenAI /v1/chat/completions (router/proxy, OpenRouter, ...)

export interface OpenAICompatibleOptions {
  name: string;
  baseUrl: string;
  apiKey: string;
  model: string;
}

interface ChatCompletionResponse {
  choices?: { message?: { content?: string | { type: string; text?: string }[] } }[];
}

export class OpenAICompatibleProvider implements TranslationProvider {
  readonly name: string;
  private readonly options: OpenAICompatibleOptions;

  constructor(options: OpenAICompatibleOptions) {
    this.name = options.name;
    this.options = options;
  }

  async translate(request: TranslateRequest): Promise<TextLine[]> {
    const { width, height } = request;
    const dataUrl = `data:${request.mimeType};base64,${Buffer.from(request.image).toString("base64")}`;

    const response = await postJson<ChatCompletionResponse>(
      this.name,
      `${this.options.baseUrl.replace(/\/+$/, "")}/chat/completions`,
      { authorization: `Bearer ${this.options.apiKey}` },
      {
        model: this.options.model,
        max_tokens: 4000,
        messages: [
          {
            role: "user",
            content: [
              { type: "image_url", image_url: { url: dataUrl } },
              { type: "text", text: buildPrompt(request.targetLang, width, height, "pixels") },
            ],
          },
        ],
      },
      request.signal,
    );

    const content = response.choices?.[0]?.message?.content;
    const text = Array.isArray(content) ? content.map((part) => part.text ?? "").join("") : content;
    if (!text)
      throw new ProviderError(this.name, "empty response");

    try {
      return normalizeLines(extractJson(text), width, height, "pixels");
    } catch (error) {
      throw new ProviderError(this.name, `invalid output: ${(error as Error).message}`);
    }
  }
}
