// Danh sách provider theo thứ tự ưu tiên. Không chứa secret: key/URL đọc từ biến môi trường.

export type ProviderType = "openai-compatible" | "gemini";

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
        model: env.GEMINI_MODEL_PRIMARY ?? "gemini-3.5-flash-lite",
      },
      {
        name: "gemini-fallback",
        type: "gemini",
        priority: 3,
        apiKey: env.GEMINI_API_KEY,
        model: env.GEMINI_MODEL_FALLBACK ?? "gemini-3-flash-preview",
      },
    ],
  };
}
