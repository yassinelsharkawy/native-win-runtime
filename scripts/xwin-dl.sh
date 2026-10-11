#!/bin/sh
set -eu

XWIN_CACHE="${XDG_CACHE_HOME:-$HOME/.cache}/xwin"
XWIN_DIR="$HOME/.local/share/xwin"
XWIN_DL="$XWIN_CACHE/download"

mkdir -p "$XWIN_CACHE"
mkdir -p "$XWIN_DIR"
mkdir -p "$XWIN_DL"

ARCHIVE="$XWIN_DL/xwin.tar.gz"

curl -L \
  https://github.com/Jake-Shadle/xwin/releases/download/0.9.0/xwin-0.9.0-x86_64-unknown-linux-musl.tar.gz \
  -o "$ARCHIVE"

tar -xzf "$ARCHIVE" -C "$XWIN_DL"

XWIN_BIN="$(find "$XWIN_DL" -type f -name xwin | head -n 1)"

[ -n "$XWIN_BIN" ] || {
    echo "xwin binary not found"
    exit 1
}

"$XWIN_BIN" \
  --accept-license \
  --cache-dir "$XWIN_CACHE" \
  --arch x86_64 \
  splat \
  --output "$XWIN_DIR"
