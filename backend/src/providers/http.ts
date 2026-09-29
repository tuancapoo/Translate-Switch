import { ProviderError } from "./types.ts";

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
    });
  } catch (error) {
    const reason = signal.aborted ? "timeout" : (error as Error).message;
    throw new ProviderError(provider, `request failed: ${reason}`);
  }

  if (!response.ok) {
    const detail = (await response.text().catch(() => "")).slice(0, 300);
    throw new ProviderError(provider, `HTTP ${response.status}: ${detail}`, response.status);
  }

  return (await response.json()) as T;
}
