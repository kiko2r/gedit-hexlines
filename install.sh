#!/bin/bash
set -e
DEST="$HOME/.local/share/gedit/plugins"
SRC="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$DEST"
cp "$SRC/hexlines.plugin" "$SRC/hexlines.py" "$DEST/"
echo "Plugin installed in $DEST"
echo "Now open gedit > Preferences > Plugins and enable 'Hexadecimal Line Numbers'."
