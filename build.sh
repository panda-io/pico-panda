#!/bin/bash
set -e
cd "$(dirname "$0")"
python3 gen_boot.sh
mpd build ppd
cp bin/ppd ~/.local/bin/ppd
echo "built and installed bin/ppd"
