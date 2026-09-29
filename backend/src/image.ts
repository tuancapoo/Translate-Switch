// Nhận diện định dạng ảnh (JPEG/PNG/WebP) qua magic bytes và đọc kích thước thật từ header

export type ImageMimeType = "image/jpeg" | "image/png" | "image/webp";

export interface ImageInfo {
  mimeType: ImageMimeType;
  width: number;
  height: number;
}

// Kích thước mặc định khi không đọc được header (ảnh chụp màn hình Switch)
const DEFAULT_SIZE = { width: 1280, height: 720 };

const PNG_SIGNATURE = [0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a];

function readPngSize(data: Uint8Array) {
  // Chunk đầu tiên luôn là IHDR: width và height (big-endian) ở byte 16 và 20
  if (data.length < 24)
    return DEFAULT_SIZE;
  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
  return { width: view.getUint32(16), height: view.getUint32(20) };
}

function readJpegSize(data: Uint8Array) {
  // Duyệt các segment tới marker SOFn (0xC0-0xCF, trừ DHT 0xC4, JPG 0xC8, DAC 0xCC)
  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
  let offset = 2;
  while (offset + 9 < data.length) {
    if (data[offset] !== 0xff) {
      offset++;
      continue;
    }
    const marker = data[offset + 1];
    const length = view.getUint16(offset + 2);
    if (marker >= 0xc0 && marker <= 0xcf && marker !== 0xc4 && marker !== 0xc8 && marker !== 0xcc)
      return { height: view.getUint16(offset + 5), width: view.getUint16(offset + 7) };
    offset += 2 + length;
  }
  return DEFAULT_SIZE;
}

const ascii = (data: Uint8Array, start: number, end: number) => String.fromCharCode(...data.subarray(start, end));

function readWebpSize(data: Uint8Array) {
  // RIFF....WEBP rồi tới chunk đầu tiên ở byte 12: "VP8 " (lossy), "VP8L" (lossless) hoặc "VP8X" (extended)
  if (data.length < 30)
    return DEFAULT_SIZE;
  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
  switch (ascii(data, 12, 16)) {
    case "VP8 ":
      return { width: view.getUint16(26, true) & 0x3fff, height: view.getUint16(28, true) & 0x3fff };
    case "VP8L": {
      const bits = view.getUint32(21, true);
      return { width: (bits & 0x3fff) + 1, height: ((bits >> 14) & 0x3fff) + 1 };
    }
    case "VP8X":
      return {
        width: (data[24] | (data[25] << 8) | (data[26] << 16)) + 1,
        height: (data[27] | (data[28] << 8) | (data[29] << 16)) + 1,
      };
    default:
      return DEFAULT_SIZE;
  }
}

/** Trả về undefined nếu không phải JPEG, PNG hoặc WebP */
export function detectImage(data: Uint8Array): ImageInfo | undefined {
  if (data.length >= 3 && data[0] === 0xff && data[1] === 0xd8 && data[2] === 0xff)
    return { mimeType: "image/jpeg", ...readJpegSize(data) };

  if (data.length >= PNG_SIGNATURE.length && PNG_SIGNATURE.every((byte, i) => data[i] === byte))
    return { mimeType: "image/png", ...readPngSize(data) };

  if (data.length >= 12 && ascii(data, 0, 4) === "RIFF" && ascii(data, 8, 12) === "WEBP")
    return { mimeType: "image/webp", ...readWebpSize(data) };

  return undefined;
}
