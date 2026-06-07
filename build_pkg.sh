#!/bin/bash
# IMX6Q AMP Build Script
# AI Model: Doubao 2.0 Expert Model (ByteDance)

set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
OUTPUT_DIR="$ROOT/output"
mkdir -p "$OUTPUT_DIR"

# Handle clean option
if [ $# -gt 0 ] && [ "$1" = "-c" ]; then
    rm -rf "$OUTPUT_DIR"/*
    echo "Output directory cleaned"
    exit 0
fi

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE="$OUTPUT_DIR/PKG_${TIMESTAMP}.log.txt"
PKG_FILE="$OUTPUT_DIR/PKG_${TIMESTAMP}.tar"

exec > >(tee "$LOG_FILE") 2>&1

echo "Build started: $TIMESTAMP"
echo "Output directory: $OUTPUT_DIR"

# Clean output directory
rm -rf "$OUTPUT_DIR"/*
echo "Output directory cleaned"

# Define projects and files
declare -A PROJS=(
    ["kernel_app/amp_sgi"]="amp_sgi.ko"
    ["cpu3_app"]="cpu3_app.elf"
    ["linux_app"]="SW_APP.out"
)

# Clean and build all projects
for proj in "${!PROJS[@]}"; do
    echo "Cleaning: $proj"
    cd "$ROOT/$proj" && make clean
    
    echo "Compiling: $proj"
    make -j$(nproc)
done

# Collect files
TMP_DIR="$ROOT/_pkg_tmp"
rm -rf "$TMP_DIR" && mkdir -p "$TMP_DIR"

for proj in "${!PROJS[@]}"; do
    out_dir="$ROOT/$proj/[ $proj = kernel_app/amp_sgi ] && echo out || echo output"
    cp "$ROOT/$proj/$( [ "$proj" = "kernel_app/amp_sgi" ] && echo out || echo output )/${PROJS[$proj]}" "$TMP_DIR/"
done

# Create package
cd "$TMP_DIR" && tar -cf "$PKG_FILE" ./*
rm -rf "$TMP_DIR"

# Summary
echo
echo "Build completed successfully!"
echo "Package: $PKG_FILE"
echo "Log: $LOG_FILE"
echo "Contents:"
tar -tf "$PKG_FILE"
echo "Size: $(du -h "$PKG_FILE" | cut -f1)"

echo "Clean all projects again"
for proj in "${!PROJS[@]}"; do
    echo "Cleaning: $proj"
    cd "$ROOT/$proj" && make clean
done
