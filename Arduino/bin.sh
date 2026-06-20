#!/bin/bash
# bin.sh — BroilerSmart firmware builder & OTA uploader
#
# Usage:
#   ./bin.sh              — Build 4MB + auto patch version (1.3.2 → 1.3.3)
#   ./bin.sh 2.0.0        — Build 4MB + manual version
#   ./bin.sh --dev        — Build 16MB+PSRAM ke ./dist/ (development)
#   ./bin.sh --ota-only   — Re-upload binary terakhir (auto patch)
#   ./bin.sh 2.0.0 --ota-only  — Re-upload dengan version manual

SKETCH="."
FQBN="esp32:esp32:esp32s3"
UPLOAD_KEY="ota-broilersmart-2026"
OTA_URL="http://127.0.0.1:3001/api/firmware/upload"

loading_animation() {
    echo -n "[----|      Please wait     |----]"
    while true; do
        for s in / - \\ \|; do
            printf " "
            printf "\r[$s----|      Please wait     |----$s]"
            printf " "
            sleep .1
        done
    done
}

# Detect jika argumen pertama adalah version number (x.y.z)
IS_MANUAL_VERSION=false
MANUAL_VERSION=""
if [[ "$1" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    IS_MANUAL_VERSION=true
    MANUAL_VERSION="$1"
    shift  # geser argumen, $1 sekarang jadi flag (jika ada)
fi

get_version() {
    if [ "$IS_MANUAL_VERSION" = true ]; then
        echo "$MANUAL_VERSION"
    else
        curl -s http://127.0.0.1:3001/api/firmware/latest | python3 -c "
import sys,json
d=json.load(sys.stdin)
v=d['version'].split('.')
print(f'{v[0]}.{v[1]}.{int(v[2])+1}')
" 2>/dev/null
    fi
}

# ============================================================
# MODE: --ota-only (skip build, upload binary terakhir)
# ============================================================
if [ "$1" = "--ota-only" ]; then
    BIN=$(find /home/aldo/.cache/arduino/sketches -name "Prototype.ino.bin" 2>/dev/null | head -1)
    if [ -z "$BIN" ]; then
        echo "Tidak ada binary di cache. Jalankan ./bin.sh dulu (tanpa --ota-only)."
        exit 1
    fi
    VERSION=$(get_version)
    if [ -z "$VERSION" ]; then
        echo "Gagal cek version terakhir (proxy mati?)"
        exit 1
    fi
    echo "Upload sebagai v$VERSION..."
    curl -s -X POST $OTA_URL \
        -H "X-Upload-Key: $UPLOAD_KEY" \
        -F "file=@$BIN" \
        -F "version=$VERSION" \
        -F "notes=OTA re-upload" | python3 -m json.tool
    exit $?
fi

# ============================================================
# MODE: --dev (16MB + PSRAM, output ke ./dist/)
# ============================================================
if [ "$1" = "--dev" ]; then
    echo "=============================================="
    echo " Mode DEVELOPMENT — 16MB Flash + PSRAM OPI"
    echo "=============================================="
    OUT_DIR="./dist"
    PROPS="FlashSize=16M,PSRAM=opi,PartitionScheme=aldo,UploadSpeed=115200,CPUFreq=240"
    loading_animation &
    LOADING_PID=$!
    arduino-cli compile --fqbn $FQBN --board-options $PROPS --output-dir $OUT_DIR $SKETCH
    kill $LOADING_PID 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "File .bin siap di: $OUT_DIR/"
    else
        echo "Gagal mengompilasi kode."
    fi
    exit $?
fi

# ============================================================
# MODE: Production (4MB + OTA upload)
# ============================================================
echo "=============================================="
echo " Build PRODUKSI — 4MB Flash (tanpa PSRAM)"
echo "=============================================="

# 1. Hapus cache
echo ">>> Hapus cache arduino..."
rm -rf /home/aldo/.cache/arduino/sketches/FC6856A2202AE5B582365BE7FE72A5FB/

# 2. Build 4MB
echo ">>> Compile firmware..."
loading_animation &
LOADING_PID=$!
arduino-cli compile --fqbn $FQBN \
    --board-options "FlashSize=4M,UploadSpeed=115200,CPUFreq=240" \
    $SKETCH 2>&1 | tail -3
BUILD_OK=$?
kill $LOADING_PID 2>/dev/null

if [ $BUILD_OK -ne 0 ]; then
    echo "Build gagal!"
    exit 1
fi

# 3. Cari binary
BIN=$(find /home/aldo/.cache/arduino/sketches -name "Prototype.ino.bin" 2>/dev/null | head -1)
if [ -z "$BIN" ]; then
    echo "Binary tidak ditemukan di cache!"
    exit 1
fi

# 4. Tentukan version: manual atau auto-increment
VERSION=$(get_version)
if [ -z "$VERSION" ]; then
    echo "Gagal cek version terakhir (proxy mati?)"
    exit 1
fi

echo ""
echo "=============================================="
echo " Upload OTA sebagai v$VERSION"
echo " File: $BIN"
echo "=============================================="

# 5. Upload via local proxy
curl -s -X POST $OTA_URL \
    -H "X-Upload-Key: $UPLOAD_KEY" \
    -F "file=@$BIN" \
    -F "version=$VERSION" \
    -F "notes=OTA build $(date '+%Y-%m-%d %H:%M')" | python3 -m json.tool

UPLOAD_OK=$?
if [ $UPLOAD_OK -eq 0 ]; then
    echo ""
    echo "=============================================="
    echo " ✅ OTA v$VERSION terupload!"
    echo "    ESP32 akan update dalam ≤5 menit"
    echo "=============================================="
else
    echo "Upload GAGAL."
fi
