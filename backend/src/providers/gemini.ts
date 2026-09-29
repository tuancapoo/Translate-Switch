import { buildPrompt, extractJson, normalizeLines } from "../prompt.ts";
import { postJson } from "./http.ts";
import { ProviderError, type TextLine, type TranslateRequest, type TranslationProvider } from "./types.ts";

// Adapter cho Gemini API (REST generateContent)

const GEMINI_BASE_URL = "https://generativelanguage.googleapis.com/v1beta";

export interface GeminiOptions {
  name: string;
  apiKey: string;
  model: string;
}

interface GenerateContentResponse {
  candidates?: { content?: { parts?: { text?: string; thought?: boolean }[] }; finishReason?: string }[];
  promptFeedback?: { blockReason?: string };
}

export class GeminiProvider implements TranslationProvider {
  readonly name: string;
  private readonly options: GeminiOptions;

  constructor(options: GeminiOptions) {
    this.name = options.name;
    this.options = options;
  }

  async translate(request: TranslateRequest): Promise<TextLine[]> {
    const { width, height } = request;
    // Cho phép viết model dạng "gemini/gemini-3-flash-preview"
    const model = this.options.model.replace(/^gemini\//, "");

    const response = await postJson<GenerateContentResponse>(
      this.name,
      `${GEMINI_BASE_URL}/models/${encodeURIComponent(model)}:generateContent`,
      { "x-goog-api-key": this.options.apiKey },
      {
        contents: [
          {
            role: "user",
            parts: [
              { inlineData: { mimeType: request.mimeType, data: Buffer.from(request.image).toString("base64") } },
              // Gemini xác định vị trí tốt nhất với box_2d chuẩn hoá 0-1000
              { text: buildPrompt(request.targetLang, width, height, "box_2d") },
            ],
          },
        ],
        generationConfig: {
          responseMimeType: "application/json",
          temperature: 0.2,
        },
      },
      request.signal,
    );

    if (response.promptFeedback?.blockReason)
      throw new ProviderError(this.name, `blocked: ${response.promptFeedback.blockReason}`);

    const text = response.candidates?.[0]?.content?.parts
      ?.filter((part) => !part.thought)
      .map((part) => part.text ?? "")
      .join("");
    if (!text)
      throw new ProviderError(this.name, `empty response (${response.candidates?.[0]?.finishReason ?? "no candidates"})`);

    try {
      return normalizeLines(extractJson(text), width, height, "box_2d");
    } catch (error) {
      throw new ProviderError(this.name, `invalid output: ${(error as Error).message}`);
    }
  }
}
