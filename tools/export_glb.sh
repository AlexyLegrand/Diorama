#!/bin/sh
set -e
if [ $# -lt 2 ]; then
    echo "usage: tools/export_glb.sh FILE.blend OUTPUT.glb [OBJECT...]" >&2
    exit 1
fi
TOOLS=$(cd "$(dirname "$0")" && pwd)
DEPS="$TOOLS/.deps"
if [ ! -d "$DEPS/numpy" ]; then
    uv pip install --quiet --target "$DEPS" --python "$(command -v python3)" numpy
fi
BLEND=$1
OUTPUT=$2
shift 2
PYTHONPATH="$DEPS" blender -b "$BLEND" --python "$TOOLS/blender_export.py" -- "$OUTPUT" "$@" >/dev/null
python3 "$TOOLS/glb_info.py" "$OUTPUT"
