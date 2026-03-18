#!/bin/bash
set -e
cd "$(dirname "$0")"
python3 gen_boot.sh
mpd build ppd
echo "built bin/ppd"
