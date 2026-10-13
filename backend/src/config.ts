// Danh sách provider theo thứ tự ưu tiên. Không chứa secret: key/URL đọc từ biến môi trường.

export type ProviderType = "openai-compatible" | "gemini" | "anthropic";

/** Model mặc định khi không chỉ định (nhanh/rẻ cho việc dịch ảnh, đổi được bằng env hoặc header x-model) */
export const DEFAULT_MODELS = {
  gemini: "gemini-3.5-flash-lite",
  anthropic: "claude-opus-5-5",
  openai: "gpt-6-luna",
} as const;

export const OPENAI_BASE_URL = "https://api.openai.com/v1";

export interface ProviderConfig {
  name: string;
  type: ProviderType;
  /** Số nhỏ chạy trước; lỗi thì fallback sang số lớn hơn */
  priority: number;
  model?: string;
  apiKey?: string;
  baseUrl?: string;
}

export interface AppConfig {
  appToken?: string;
  /** Thời gian tối đa cho một provider trước khi chuyển sang provider sau */
  providerTimeoutMs: number;
  /** Tổng thời gian cho cả chuỗi fallback (phải nhỏ hơn giới hạn của Netlify Function) */
  totalTimeoutMs: number;
  providers: ProviderConfig[];
}

const toInt = (value: string | undefined, fallback: number) => {
  const parsed = Number.parseInt(value ?? "", 10);
  return Number.isFinite(parsed) ? parsed : fallback;
};

export function loadConfig(env: NodeJS.ProcessEnv = process.env): AppConfig {
  return {
    appToken: env.APP_TOKEN,
    providerTimeoutMs: toInt(env.PROVIDER_TIMEOUT_MS, 12_000),
    totalTimeoutMs: toInt(env.TOTAL_TIMEOUT_MS, 25_000),
    providers: [
      {
        name: "router",
        type: "openai-compatible",
        priority: 2,
        baseUrl: env.ROUTER_BASE_URL,
        apiKey: env.ROUTER_API_KEY,
        model: env.ROUTER_MODEL,
      },
      {
        name: "gemini-primary",
        type: "gemini",
        priority: 1,
        apiKey: env.GEMINI_API_KEY,
        model: env.GEMINI_MODEL_PRIMARY ?? DEFAULT_MODELS.gemini,
      },
      {
        name: "gemini-fallback",
        type: "gemini",
        priority: 3,
        apiKey: env.GEMINI_API_KEY,
        model: env.GEMINI_MODEL_FALLBACK ?? "gemini-3-flash-preview",
      },
      // Provider tuỳ chọn: chỉ chạy khi có API key tương ứng, xếp sau các provider ở trên
      {
        name: "claude",
        type: "anthropic",
        priority: 4,
        apiKey: env.ANTHROPIC_API_KEY,
        model: env.ANTHROPIC_MODEL ?? DEFAULT_MODELS.anthropic,
      },
      {
        name: "openai",
        type: "openai-compatible",
        priority: 5,
        baseUrl: OPENAI_BASE_URL,
        apiKey: env.OPENAI_API_KEY,
        model: env.OPENAI_MODEL ?? DEFAULT_MODELS.openai,
      },
    ],
  };
}
