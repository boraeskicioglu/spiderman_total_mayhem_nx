#!/bin/sh
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
LAUNCHER_DIR="$HERE" PAYLOAD=spiderman_tm_nx exec "$HERE/../runtime/launcher/build.sh" "$@"
