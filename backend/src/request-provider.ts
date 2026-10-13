import { lookup } from "node:dns/promises";
import { isIP } from "node:net";
import { DEFAULT_MODELS, OPENAI_BASE_URL, type ProviderConfig } from "./config.ts";

// Provider do client chọn cho từng request (thay vì cấu hình trong env):
//   x-provider  gemini | claude (anthropic) | openai (gpt) | openrouter | openai-compatible
//   x-api-key   key của chính người dùng
//   x-model     tuỳ chọn với gemini, claude, openai (có model mặc định), bắt buộc với openrouter và openai-compatible
//   x-base-url  chỉ dùng với openai-compatible (URL https công khai, kết thúc bằng /v1)

/** Lỗi do request sai: trả về 400 kèm thông báo */
export class RequestError extends Error {}

interface Preset {
  type: ProviderConfig["type"];
  baseUrl?: string;
  defaultModel?: string;
}

const GEMINI: Preset = { type: "gemini", defaultModel: DEFAULT_MODELS.gemini };
const CLAUDE: Preset = { type: "anthropic", defaultModel: DEFAULT_MODELS.anthropic };
const OPENAI: Preset = { type: "openai-compatible", baseUrl: OPENAI_BASE_URL, defaultModel: DEFAULT_MODELS.openai };

const PRESETS: Record<string, Preset> = {
  gemini: GEMINI,
  claude: CLAUDE,
  anthropic: CLAUDE,
  openai: OPENAI,
  gpt: OPENAI,
  openrouter: { type: "openai-compatible", baseUrl: "https://openrouter.ai/api/v1" },
  "openai-compatible": { type: "openai-compatible" },
};

const MAX_KEY_LENGTH = 512;
const MODEL_PATTERN = /^[A-Za-z0-9._:/-]{1,100}$/;

/** Trả về undefined nếu request không chỉ định provider (dùng cấu hình của server) */
export async function parseProviderOverride(headers: Headers): Promise<ProviderConfig | undefined> {
  const providerName = headers.get("x-provider")?.trim().toLowerCase();
  if (!providerName)
    return undefined;

  const preset = PRESETS[providerName];
  if (!preset)
    throw new RequestError(`Unknown x-provider "${providerName.slice(0, 40)}". Use: ${Object.keys(PRESETS).join(", ")}`);

  const apiKey = headers.get("x-api-key")?.trim();
  if (!apiKey)
    throw new RequestError("x-api-key is required when x-provider is set");
  if (apiKey.length > MAX_KEY_LENGTH || /[\s\u0000-\u001f]/.test(apiKey))
    throw new RequestError("x-api-key is malformed");

  const model = headers.get("x-model")?.trim() || preset.defaultModel;
  if (!model)
    throw new RequestError(`x-model is required for provider "${providerName}"`);
  if (!MODEL_PATTERN.test(model))
    throw new RequestError("x-model is malformed");

  let baseUrl = preset.baseUrl;
  if (preset.type === "openai-compatible") {
    const requested = headers.get("x-base-url")?.trim();
    if (requested && providerName !== "openai-compatible")
      throw new RequestError(`x-base-url is only allowed with x-provider: openai-compatible`);
    baseUrl = requested || baseUrl;
    if (!baseUrl)
      throw new RequestError("x-base-url is required for provider openai-compatible");
    await assertPublicHttpsUrl(baseUrl);
  } else if (headers.get("x-base-url")) {
    throw new RequestError("x-base-url is not supported for this provider");
  }

  return { name: `custom-${providerName}`, type: preset.type, priority: 0, apiKey, model, baseUrl };
}

// --- Chống SSRF: server sẽ gọi tới URL do client đưa nên chỉ cho phép địa chỉ công khai ---

function isPrivateAddress(address: string): boolean {
  if (isIP(address) === 4) {
    const [a, b] = address.split(".").map(Number);
    return (
      a === 0 || a === 10 || a === 127 ||
      (a === 100 && b >= 64 && b <= 127) ||   // CGNAT
      (a === 169 && b === 254) ||              // link-local, metadata cloud
      (a === 172 && b >= 16 && b <= 31) ||
      (a === 192 && b === 168) ||
      a >= 224                                  // multicast, reserved
    );
  }

  const lower = address.toLowerCase();

  // IPv4 nhúng trong IPv6 (::ffff:a.b.c.d). URL() chuẩn hoá thành dạng hex ::ffff:7f00:1 nên phải xử lý cả hai
  const mapped = lower.match(/^::ffff:(?:(\d+\.\d+\.\d+\.\d+)|([0-9a-f]{1,4}):([0-9a-f]{1,4}))$/);
  if (mapped) {
    if (mapped[1])
      return isPrivateAddress(mapped[1]);
    const high = Number.parseInt(mapped[2], 16);
    const low = Number.parseInt(mapped[3], 16);
    return isPrivateAddress(`${high >> 8}.${high & 0xff}.${low >> 8}.${low & 0xff}`);
  }

  return lower === "::" || lower === "::1" || /^f[cd]/.test(lower) || /^fe[89ab]/.test(lower);
}

export async function assertPublicHttpsUrl(raw: string): Promise<void> {
  let url: URL;
  try {
    url = new URL(raw);
  } catch {
    throw new RequestError("x-base-url is not a valid URL");
  }

  if (url.protocol !== "https:")
    throw new RequestError("x-base-url must use https");
  if (url.username || url.password)
    throw new RequestError("x-base-url must not contain credentials");
  if (url.port && url.port !== "443")
    throw new RequestError("x-base-url must use the default https port");

  const host = url.hostname.replace(/^\[|\]$/g, "").toLowerCase();
  if (host === "localhost" || host.endsWith(".localhost") || host.endsWith(".local") || host.endsWith(".internal"))
    throw new RequestError("x-base-url must be a public host");

  const addresses = isIP(host)
    ? [host]
    : await lookup(host, { all: true }).then((records) => records.map((r) => r.address)).catch(() => {
        throw new RequestError("x-base-url host cannot be resolved");
      });

  if (addresses.length === 0 || addresses.some(isPrivateAddress))
    throw new RequestError("x-base-url must be a public host");
}

/** Xoá API key khỏi chuỗi trước khi trả về hoặc ghi log */
export function redactSecret(text: string, secret: string | undefined): string {
  return secret ? text.split(secret).join("***") : text;
}
