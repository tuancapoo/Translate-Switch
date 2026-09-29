// Test nhanh trên PC, không cần deploy (đọc key từ backend/.env):
//   npm run test:image -- D:\path\to\capture.jpg   (hoặc .png, .webp)

import { readFileSync } from "node:fs";
import { loadConfig } from "../src/config.ts";
import { detectImage } from "../src/image.ts";
import { createProviders } from "../src/providers/factory.ts";
import { FallbackTranslator } from "../src/translator.ts";

const path = process.argv[2];
if (!path) {
  console.error("Usage: npm run test:image -- <file.jpg>");
  process.exit(1);
}

const config = loadConfig();
const translator = new FallbackTranslator(createProviders(config.providers), config);

const image = readFileSync(path);
const info = detectImage(image);
if (!info) {
  console.error("File must be a JPEG, PNG or WebP image");
  process.exit(1);
}

const result = await translator.translate(image, info, "Vietnamese");
console.log(JSON.stringify(result, null, 2));
