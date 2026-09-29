# Tạo thư mục release/CP-Translate với cấu trúc thẻ SD, chép thẳng vào gốc thẻ SD là dùng được.
#   powershell -ExecutionPolicy Bypass -File tools\make-release.ps1
#
# Font (tuỳ chọn): đặt DejaVuSans.ttf (+ LICENSE của DejaVu) vào tools\release-assets\ để đóng gói kèm.

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

# config.ini mẫu: không chứa token thật
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
[IO.File]::WriteAllText((Join-Path $dataDir "config.ini"), @"
# CP-Translate - cấu hình
# Điền url backend và token (APP_TOKEN của backend) rồi lưu lại.

url=https://<ten-site>.netlify.app/api/translate
token=
lang=Vietnamese

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
    $fontNote = "Đã kèm font DejaVuSans.ttf."
} else {
    $fontNote = "Chưa kèm font: tải DejaVuSans.ttf tại https://dejavu-fonts.github.io/Download.html và chép vào config\translate\"
    Write-Warning "Không có tools\release-assets\DejaVuSans.ttf - release sẽ không kèm font tiếng Việt"
}

[IO.File]::WriteAllText((Join-Path $out "README.txt"), @"
CP-Translate - overlay dịch màn hình cho Nintendo Switch (Atmosphère + Ultrahand/Tesla)
=====================================================================================

CÀI ĐẶT
1. Chép TOÀN BỘ nội dung thư mục này vào GỐC thẻ SD (gộp với thư mục switch\ và config\ sẵn có).
2. Mở config\translate\config.ini, điền url và token của backend.
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
