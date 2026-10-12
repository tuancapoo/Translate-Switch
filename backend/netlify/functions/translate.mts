import type { Config } from "@netlify/functions";
import { timingSafeEqual } from "node:crypto";
import { loadConfig } from "../../src/config.ts";
import { detectImage } from "../../src/image.ts";
import { createProviders } from "../../src/providers/factory.ts";
import { parseProviderOverride, redactSecret, RequestError } from "../../src/request-provider.ts";
import { AllProvidersFailedError, FallbackTranslator } from "../../src/translator.ts";

// POST /api/translate
//   Header  x-app-token: <APP_TOKEN>     bắt buộc
//   Header  x-target-lang: Vietnamese     tuỳ chọn
//   Header  x-provider / x-api-key / x-model / x-base-url   tuỳ chọn: tự chọn provider (xem request-provider.ts)
//   Body    ảnh JPEG, PNG hoặc WebP thô (Content-Type: image/jpeg | image/png | image/webp)
// 200 →    { lines: [{ x, y, w, h, source, translation }], provider, attempts }

const MAX_IMAGE_BYTES = 2 * 1024 * 1024;

// Tạo một lần cho mỗi instance, dùng lại giữa các request khi function còn "ấm"
const appConfig = loadConfig();
const translator = new FallbackTranslator(createProviders(appConfig.providers), appConfig);

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

  // Client có thể tự chọn provider + API key; khi đó chỉ dùng provider đó, không đụng tới key của server
  let activeTranslator = translator;
  const apiKey = req.headers.get("x-api-key")?.trim();
  try {
    const override = await parseProviderOverride(req.headers);
    if (override)
      activeTranslator = new FallbackTranslator(createProviders([override]), appConfig);
  } catch (error) {
    if (error instanceof RequestError)
      return json(400, { error: error.message });
    throw error;
  }

  try {
    return json(200, await activeTranslator.translate(image, info, targetLang));
  } catch (error) {
    if (error instanceof AllProvidersFailedError) {
      const attempts = error.attempts.map((a) => ({ ...a, error: a.error && redactSecret(a.error, apiKey) }));
      return json(502, { error: "All providers failed", attempts });
    }
    console.error(redactSecret(String(error), apiKey));
    return json(500, { error: "Internal error" });
  }
};

export const config: Config = {
  path: "/api/translate",
};
