#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="${1:-$HOME/.local/bin}"
TARGET_BIN="$TARGET_DIR/ge"

mkdir -p "$TARGET_DIR"

echo "[ge-install] building GE interpreter..."
EXTRA_LIBS=""
if [[ "$(uname -s)" == "Linux" ]]; then
    EXTRA_LIBS="-lX11 -lGL"
fi

g++ -std=c++17 -O2 "$ROOT_DIR/interpreter/run.cpp" -o "$TARGET_BIN" $EXTRA_LIBS
chmod +x "$TARGET_BIN"

echo "[ge-install] installed: $TARGET_BIN"

echo "[ge-install] if 'ge' is not found, add this to your shell profile:"
echo "export PATH=\"$TARGET_DIR:\$PATH\""
