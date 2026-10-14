# Tạo thư mục release/CP-Translate với cấu trúc thẻ SD, chép thẳng vào gốc thẻ SD là dùng được.
#   powershell -ExecutionPolicy Bypass -File tools\make-release.ps1 -Token <APP_TOKEN>
#
# -Url   địa chỉ backend điền sẵn vào config.ini (mặc định: site đang dùng)
# -Token APP_TOKEN điền sẵn vào config.ini; hoặc đặt biến môi trường RELEASE_APP_TOKEN. Bỏ trống thì để trống.
#        Token KHÔNG được ghi vào script này hay vào git: nó chỉ có trong gói zip phát hành.
#
# Font (tuỳ chọn): đặt DejaVuSans.ttf (+ LICENSE của DejaVu) vào tools\release-assets\ để đóng gói kèm.

param(
    [string]$Url = "https://translate-nx.netlify.app/api/translate",
    [string]$Token = $env:RELEASE_APP_TOKEN
)

$ErrorActionPreference = "Stop"

$root    = Split-Path -Parent $PSScriptRoot
$ovl     = Join-Path $root "overlay\translate.ovl"
$assets  = Join-Path $PSScriptRoot "release-assets"
$out     = Join-Path $root "release\CP-Translate"

if (-not (Test-Path $ovl)) {
    throw "Không thấy overlay\translate.ovl. Build trước: cd overlay; make"
}

# Tạo lại thư mục sạch
if (Test-Path $out) { Remove-Item -LiteralPath $out -Recurse -Force }
$overlays = New-Item -ItemType Directory -Force (Join-Path $out "switch\.overlays")
$dataDir  = New-Item -ItemType Directory -Force (Join-Path $out "config\translate")

Copy-Item $ovl $overlays

# config.ini: url và token được điền sẵn nếu truyền vào; người dùng chỉ cần điền api_key
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
if (-not $Token) { Write-Warning "Không có -Token: config.ini trong gói sẽ để trống token" }
[IO.File]::WriteAllText((Join-Path $dataDir "config.ini"), @"
# CP-Translate - cấu hình
# url và token của backend đã được điền sẵn. Bạn chỉ cần điền api_key rồi lưu lại.

url=$Url
token=$Token
lang=Vietnamese

# Provider và API key của riêng bạn. api_key là BẮT BUỘC: server không giữ key nào, chỉ dùng key bạn gửi lên.
#
# provider: gemini | claude | openai | openrouter | openai-compatible
#   gemini             model mặc định gemini-3.5-flash-lite (nếu chậm, thử model=gemini-3.1-flash-lite)
#   claude             model mặc định claude-opus-5-5 (mạnh nhưng chậm/đắt; nhanh hơn: claude-sonnet-5-5, claude-haiku-4-5)
#   openai             model mặc định gpt-6-luna
#   openrouter         BẮT BUỘC đặt model (tên model của OpenRouter)
#   openai-compatible  BẮT BUỘC đặt model và base_url (API chuẩn /v1/chat/completions, https công khai)
# model bỏ trống = dùng model mặc định ở trên.
provider=gemini
api_key=
model=
# Chỉ dùng với provider=openai-compatible, ví dụ https://api.example.com/v1
base_url=

# Độ đậm nền box bản dịch, 0-100 (0 = trong suốt hoàn toàn, 100 = đặc). Đổi được trong overlay: Cài đặt > Nền box.
box_opacity=70

# Phím tắt chụp & dịch (2-4 nút), đổi được trong overlay: Cài đặt > Phím tắt
# Tên nút: L R ZL ZR SL SR A B X Y DUP DDOWN DLEFT DRIGHT LS RS PLUS MINUS
hotkey=L+R+ZR
"@.Replace("`r`n", "`n"), $utf8NoBom)

# Font tiếng Việt (nếu có)
$font = Join-Path $assets "DejaVuSans.ttf"
if (Test-Path $font) {
    Copy-Item $font $dataDir
    $license = Join-Path $assets "LICENSE"
    if (Test-Path $license) { Copy-Item $license (Join-Path $dataDir "DejaVu-LICENSE.txt") }
    $fontNote = "Đã kèm font DejaVuSans.ttf (giấy phép: config\translate\DejaVu-LICENSE.txt, giữ nguyên file này)."
} else {
    $fontNote = "Chưa kèm font: tải DejaVuSans.ttf tại https://dejavu-fonts.github.io/Download.html và chép vào config\translate\"
    Write-Warning "Không có tools\release-assets\DejaVuSans.ttf - release sẽ không kèm font tiếng Việt"
}

[IO.File]::WriteAllText((Join-Path $out "README.txt"), @"
CP-Translate - overlay dịch màn hình cho Nintendo Switch (Atmosphère + Ultrahand/Tesla)
=====================================================================================

CÀI ĐẶT
1. Chép TOÀN BỘ nội dung thư mục này vào GỐC thẻ SD (gộp với thư mục switch\ và config\ sẵn có).
2. Mở config\translate\config.ini: url và token đã điền sẵn, bạn chỉ cần điền api_key (provider mặc định là gemini; đổi provider và model trong cùng file, xem chú thích bên trong).
3. $fontNote

Cấu trúc sau khi chép:
  SD:\switch\.overlays\translate.ovl
  SD:\config\translate\config.ini
  SD:\config\translate\DejaVuSans.ttf

SỬ DỤNG
- Vào game, mở menu overlay (Ultrahand mặc định ZL + ZR + xuống), chọn Translate, bấm B để ẩn.
- Bấm L + R + ZR để chụp và dịch. Bản dịch hiện đè lên màn hình.
- Chạm màn hình hoặc bấm nút bất kỳ để ẩn bản dịch.

Yêu cầu: Atmosphère, Ultrahand Overlay (hoặc Tesla Menu + nx-ovlloader), Wi-Fi có Internet.
"@.Replace("`n", "`r`n").Replace("`r`r`n", "`r`n"), $utf8NoBom)

Write-Host "Đã tạo: $out"
Get-ChildItem $out -Recurse -File | ForEach-Object { "  " + $_.FullName.Substring($out.Length + 1) + "  (" + [math]::Round($_.Length / 1KB) + " KB)" }
