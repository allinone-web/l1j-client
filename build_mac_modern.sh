#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build_modern"
LIBS_DIR="$ROOT_DIR/libs"
ARCH="${ARCH:-x86_64}"

echo "Root: $ROOT_DIR"
echo "Build dir: $BUILD_DIR"
echo "Arch: $ARCH"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running configure..."
"$ROOT_DIR/configure" \
  LDFLAGS="-F$LIBS_DIR -Wl,-rpath,$LIBS_DIR -arch $ARCH" \
  CPPFLAGS="-I$LIBS_DIR/SDL.framework/Headers -I$LIBS_DIR/SDL_image.framework/Headers -I$LIBS_DIR/SDL_mixer.framework/Headers -arch $ARCH" \
  CFLAGS="-arch $ARCH" \
  CXXFLAGS="-arch $ARCH" \
  OBJCFLAGS="-arch $ARCH"

echo "Building..."
make -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "Packaging client.app via upstream target..."
make mac_test

rm -rf Lineage.app
cp -R client.app Lineage.app

if [ -f "Lineage.app/Contents/Info.PList" ]; then
  mv "Lineage.app/Contents/Info.PList" "Lineage.app/Contents/Info.plist"
fi

if [ -n "${ASSETS_PATH:-}" ]; then
  if [ -d "$ASSETS_PATH" ]; then
    sed -i '' "s#^Path = .*#Path = $ASSETS_PATH/#" Lineage.app/Contents/Resources/Lineage.ini
    echo "Updated Lineage.ini Path to: $ASSETS_PATH/"
  else
    echo "ASSETS_PATH does not exist: $ASSETS_PATH"
    exit 1
  fi
fi

echo "Build complete: $BUILD_DIR/Lineage.app"
echo "If you want deployment, run:"
echo "  cp -R \"$BUILD_DIR/Lineage.app\" \"/Users/airtan/Documents/GitHub/Lineageclient-charlie/Assets/Lineage.app\""
