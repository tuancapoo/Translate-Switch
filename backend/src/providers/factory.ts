import type { ProviderConfig, ProviderType } from "../config.ts";
import { GeminiProvider } from "./gemini.ts";
import { OpenAICompatibleProvider } from "./openai-compatible.ts";
import type { TranslationProvider } from "./types.ts";

// Factory: từ cấu hình tạo ra adapter tương ứng. Thêm loại provider mới = thêm một entry vào đây.

type Builder = (config: ProviderConfig) => TranslationProvider | undefined;

const builders: Record<ProviderType, Builder> = {
  "openai-compatible": ({ name, baseUrl, apiKey, model }) =>
    baseUrl && apiKey && model ? new OpenAICompatibleProvider({ name, baseUrl, apiKey, model }) : undefined,

  gemini: ({ name, apiKey, model }) =>
    apiKey && model ? new GeminiProvider({ name, apiKey, model }) : undefined,
};

/** Tạo các provider đã cấu hình đủ, sắp xếp theo priority. Provider thiếu key/model bị bỏ qua. */
export function createProviders(configs: ProviderConfig[]): TranslationProvider[] {
  return [...configs]
    .sort((a, b) => a.priority - b.priority)
    .flatMap((config) => {
      const provider = builders[config.type](config);
      if (!provider)
        console.warn(`[factory] skip provider "${config.name}": missing key, model or base URL`);
      return provider ? [provider] : [];
    });
}
