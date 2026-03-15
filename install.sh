#!/usr/bin/env bash
set -e

INSTALL_DIR="${1:-$HOME/.local/bin}"
mkdir -p "$INSTALL_DIR"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Building ppd..."
cd "$SCRIPT_DIR"
mpd build ppd

echo "Installing to $INSTALL_DIR/ppd..."
cp bin/ppd "$INSTALL_DIR/ppd"
chmod +x "$INSTALL_DIR/ppd"

echo "Done. Run 'ppd --help' to verify."
