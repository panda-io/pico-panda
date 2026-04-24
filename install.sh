#!/usr/bin/env bash
set -e

INSTALL_DIR="${1:-$HOME/.local/bin}"
mkdir -p "$INSTALL_DIR"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Building ..."
cd "$SCRIPT_DIR"
mpd build

echo "Installing to $INSTALL_DIR ..."
cp bin/ppd "$INSTALL_DIR/ppd"
chmod +x "$INSTALL_DIR/ppd"
codesign --force --sign - "$INSTALL_DIR/ppd" 2>/dev/null || true

cp bin/panda "$INSTALL_DIR/panda"
chmod +x "$INSTALL_DIR/panda"
codesign --force --sign - "$INSTALL_DIR/panda" 2>/dev/null || true

echo "Done. Run 'ppd --help' to verify."
