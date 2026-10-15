import type { Config } from "@netlify/functions";
import { timingSafeEqual } from "node:crypto";
import { loadConfig } from "../../src/config.ts";
import { detectImage } from "../../src/image.ts";
import { createProviders } from "../../src/providers/factory.ts";
import { parseProviderOverride, redactSecret, RequestError, withGeminiFallback } from "../../src/request-provider.ts";
import { AllProvidersFailedError, FallbackTranslator } from "../../src/translator.ts";

// POST /api/translate
//   Header  x-app-token: <APP_TOKEN>      bắt buộc
//   Header  x-provider: gemini | claude | openai | openrouter | openai-compatible   bắt buộc
//   Header  x-api-key: <key của người dùng>                                          bắt buộc
//   Header  x-model / x-base-url          tuỳ chọn (xem request-provider.ts)
//   Header  x-target-lang: Vietnamese     tuỳ chọn
//   Body    ảnh JPEG, PNG hoặc WebP thô (Content-Type: image/jpeg | image/png | image/webp)
// 200 →    { lines: [{ x, y, w, h, source, translation }], provider, attempts }
//
// Chỉ dùng provider/key/model do người dùng gửi lên. Muốn cho phép chạy bằng key cấu hình trên server
// (khi request không có x-provider) thì đặt biến môi trường ALLOW_SERVER_PROVIDERS=true.

const MAX_IMAGE_BYTES = 2 * 1024 * 1024;

const appConfig = loadConfig();

// Chuỗi provider của server chỉ được dựng khi bật ALLOW_SERVER_PROVIDERS
const serverTranslator = appConfig.allowServerProviders
  ? new FallbackTranslator(createProviders(appConfig.providers), appConfig)
  : undefined;

function json(status: number, body: unknown): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "content-type": "application/json; charset=utf-8" },
  });
}

function isValidToken(actual: string | null): boolean {
  if (!appConfig.appToken || !actual)
    return false;
  const a = Buffer.from(actual);
  const b = Buffer.from(appConfig.appToken);
  return a.length === b.length && timingSafeEqual(a, b);
}

export default async (req: Request): Promise<Response> => {
  if (req.method !== "POST")
    return json(405, { error: "Use POST" });
  if (!appConfig.appToken)
    return json(500, { error: "Server missing APP_TOKEN" });
  if (!isValidToken(req.headers.get("x-app-token")))
    return json(401, { error: "Invalid app token" });

  const image = new Uint8Array(await req.arrayBuffer());
  if (image.length > MAX_IMAGE_BYTES)
    return json(413, { error: "Image too large" });

  const info = detectImage(image);
  if (!info)
    return json(415, {
      error: "Body must be a JPEG, PNG or WebP image",
      // Thông tin để debug: request thật sự gửi lên cái gì
      received: {
        bytes: image.length,
        contentType: req.headers.get("content-type"),
        firstBytes: Buffer.from(image.subarray(0, 16)).toString("hex"),
      },
    });

  const targetLang = req.headers.get("x-target-lang") || "Vietnamese";
  const apiKey = req.headers.get("x-api-key")?.trim();

  // Provider do người dùng chọn (kèm API key và model của họ)
  let override;
  try {
    override = await parseProviderOverride(req.headers);
  } catch (error) {
    if (error instanceof RequestError)
      return json(400, { error: error.message });
    throw error;
  }

  if (!override && !serverTranslator)
    return json(400, { error: "x-provider and x-api-key are required (server providers are disabled)" });

  // Gemini: nếu model người dùng chọn lỗi (503, timeout...) thì tự thử gemini-3.5-flash-lite với cùng key
  const translator = override
    ? new FallbackTranslator(createProviders(withGeminiFallback(override)), appConfig)
    : serverTranslator!;

  try {
    return json(200, await translator.translate(image, info, targetLang));
  } catch (error) {
    if (error instanceof AllProvidersFailedError) {
      const attempts = error.attempts.map((a) => ({ ...a, error: a.error && redactSecret(a.error, apiKey) }));
      // Một provider do người dùng chọn: nói rõ nó lỗi gì thay vì "All providers failed"
      if (override)
        return json(502, {
          error: "Provider request failed",
          provider: override.name,
          detail: attempts.map((a) => a.error).join(" | ").slice(0, 600),
          attempts,
        });
      return json(502, { error: "All providers failed", attempts });
    }
    console.error(redactSecret(String(error), apiKey));
    return json(500, { error: "Internal error" });
  }
};

export const config: Config = {
  path: "/api/translate",
};
