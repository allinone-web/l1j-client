#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build_modern"
LIBS_DIR="$ROOT_DIR/libs"
ARCH="${ARCH:-x86_64}"
DEPLOY_PATH="${DEPLOY_PATH:-}"
CLEAN_BUILD_DIR_AFTER_DEPLOY="${CLEAN_BUILD_DIR_AFTER_DEPLOY:-0}"

echo "Root: $ROOT_DIR"
echo "Build dir: $BUILD_DIR"
echo "Arch: $ARCH"
if [ -n "$DEPLOY_PATH" ]; then
  echo "Deploy path: $DEPLOY_PATH"
fi
echo "Clean build dir after deploy: $CLEAN_BUILD_DIR_AFTER_DEPLOY"

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

if [ -n "$DEPLOY_PATH" ]; then
  if [ -d "$DEPLOY_PATH" ]; then
    rm -rf "$DEPLOY_PATH/Lineage.app"
    cp -R "$BUILD_DIR/Lineage.app" "$DEPLOY_PATH/Lineage.app"
    echo "Deployed app to: $DEPLOY_PATH/Lineage.app"
  else
    echo "DEPLOY_PATH does not exist: $DEPLOY_PATH"
    exit 1
  fi
fi

if [ "$CLEAN_BUILD_DIR_AFTER_DEPLOY" = "1" ]; then
  if [ -n "$DEPLOY_PATH" ]; then
    cd "$ROOT_DIR"
    rm -rf "$BUILD_DIR"
    echo "Cleaned build directory: $BUILD_DIR"
  else
    echo "CLEAN_BUILD_DIR_AFTER_DEPLOY=1 ignored because DEPLOY_PATH is not set"
  fi
fi

echo "Build complete: $BUILD_DIR/Lineage.app"
echo "If you want deployment, run:"
echo "  cp -R \"$BUILD_DIR/Lineage.app\" \"/Users/airtan/Documents/GitHub/Lineageclient-charlie/Assets/Lineage.app\""
