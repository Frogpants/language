#!/bin/bash

set -u

SCRIPT_PATH="${1:-test.ge}"

if [ ! -f "$SCRIPT_PATH" ]; then
    echo "Error: script file not found: $SCRIPT_PATH"
    echo "Usage: ./run.sh <script.ge>"
    exit 1
fi

echo "[build] compiling ge interpreter..."
EXTRA_LIBS=""
if [ "$(uname -s)" = "Linux" ]; then
    EXTRA_LIBS="-lX11 -lGL"
fi

if ! g++ -std=c++17 interpreter/run.cpp -o /tmp/interp $EXTRA_LIBS; then
    echo "Build failed."
    exit 1
fi

echo "[run] running $SCRIPT_PATH"
if ! /tmp/interp "$SCRIPT_PATH"; then
    echo "Interpreter run failed."
    exit 1
fi
