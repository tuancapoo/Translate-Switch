import type { Config } from "@netlify/functions";
import { timingSafeEqual } from "node:crypto";
import { loadConfig } from "../../src/config.ts";
import { detectImage } from "../../src/image.ts";
import { createProviders } from "../../src/providers/factory.ts";
import { AllProvidersFailedError, FallbackTranslator } from "../../src/translator.ts";

// POST /api/translate
//   Header  x-app-token: <APP_TOKEN>     bắt buộc
//   Header  x-target-lang: Vietnamese     tuỳ chọn
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

  try {
    return json(200, await translator.translate(image, info, targetLang));
  } catch (error) {
    if (error instanceof AllProvidersFailedError)
      return json(502, { error: "All providers failed", attempts: error.attempts });
    console.error(error);
    return json(500, { error: "Internal error" });
  }
};

export const config: Config = {
  path: "/api/translate",
};
