import { ProviderError } from "./types.ts";

/** Lấy message gọn từ thân lỗi JSON ({"error":{"message":...}}); không phải JSON thì gộp khoảng trắng */
function summarizeError(text: string): string {
  try {
    const body = JSON.parse(text) as { error?: { message?: unknown } | string; message?: unknown };
    const message = typeof body.error === "string" ? body.error : (body.error?.message ?? body.message);
    if (typeof message === "string" && message.trim())
      return message.trim();
  } catch {
    // không phải JSON
  }
  return text.replace(/\s+/g, " ").trim();
}

/** POST JSON, ném ProviderError kèm status khi HTTP lỗi hoặc timeout */
export async function postJson<T>(
  provider: string,
  url: string,
  headers: Record<string, string>,
  body: unknown,
  signal: AbortSignal,
): Promise<T> {
  let response: Response;
  try {
    response = await fetch(url, {
      method: "POST",
      headers: { "content-type": "application/json", ...headers },
      body: JSON.stringify(body),
      signal,
      // Không theo redirect: URL do client chọn không được chuyển hướng sang địa chỉ nội bộ
      redirect: "error",
    });
  } catch (error) {
    const reason = signal.aborted ? "timeout" : (error as Error).message;
    throw new ProviderError(provider, `request failed: ${reason}`);
  }

  if (!response.ok) {
    const detail = summarizeError(await response.text().catch(() => "")).slice(0, 300);
    throw new ProviderError(provider, `HTTP ${response.status}: ${detail}`, response.status);
  }

  return (await response.json()) as T;
}
