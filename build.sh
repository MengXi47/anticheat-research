#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="_build"
GENERATOR="Unix Makefiles"

cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_SYSROOT=iphoneos \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR" -j 10