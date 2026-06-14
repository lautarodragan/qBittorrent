#!/usr/bin/env bash
set -euo pipefail

IMAGE="qbittorrent-builder"
CCACHE_VOLUME="qbittorrent-ccache"
SRC_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SRC_DIR/build"
CMAKE_ARGS="${@:--DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache}"

# Build image if missing or Containerfile changed since last build
if ! podman image exists "$IMAGE"; then
    REBUILD=1
else
    IMAGE_DATE=$(podman image inspect "$IMAGE" --format '{{.Created}}' 2>/dev/null || echo "0")
    CF_DATE=$(stat -c %Y "$SRC_DIR/Containerfile.build")
    IMAGE_TS=$(date -d "$IMAGE_DATE" +%s 2>/dev/null || echo 0)
    REBUILD=$(( CF_DATE > IMAGE_TS ? 1 : 0 ))
fi
if [[ $REBUILD -eq 1 ]]; then
    echo "==> Building container image..."
    podman build -f "$SRC_DIR/Containerfile.build" -t "$IMAGE" "$SRC_DIR"
fi

# Ensure ccache volume exists
podman volume exists "$CCACHE_VOLUME" || podman volume create "$CCACHE_VOLUME"

mkdir -p "$BUILD_DIR"

echo "==> Configuring..."
podman run --rm \
    -v "$SRC_DIR:/src:z" \
    -v "$CCACHE_VOLUME:/ccache:z" \
    "$IMAGE" \
    cmake -B /src/build -S /src $CMAKE_ARGS

echo "==> Building..."
podman run --rm \
    -v "$SRC_DIR:/src:z" \
    -v "$CCACHE_VOLUME:/ccache:z" \
    "$IMAGE" \
    cmake --build /src/build --parallel "$(nproc)"

echo "==> Done. Binary at: $BUILD_DIR/src/qbittorrent (or qbittorrent-nox)"
