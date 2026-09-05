#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"
WEB_ASSETS_DIR="$ROOT_DIR/.web-assets"
EMSDK_VERSION="${EMSDK_VERSION:-6.0.9}"
EMSDK_DIR="$ROOT_DIR/.emsdk"
export EMSDK_QUIET=1
trap 'rm -rf "$WEB_ASSETS_DIR"' EXIT

if ! command -v em++ >/dev/null 2>&1; then
  if [ -f "$EMSDK_DIR/emsdk_env.sh" ]; then
    # shellcheck disable=SC1091
    source "$EMSDK_DIR/emsdk_env.sh"
  fi
fi

if ! command -v em++ >/dev/null 2>&1; then
  for candidate in python3.13 python3.12 python3.11 python3.10 python3; do
    if command -v "$candidate" >/dev/null 2>&1; then
      version_ok="$("$candidate" -c 'import sys; print(int(sys.version_info >= (3, 10)))')"
      if [ "$version_ok" = "1" ]; then
        export EMSDK_PYTHON="$(command -v "$candidate")"
        break
      fi
    fi
  done

  if [ -z "${EMSDK_PYTHON:-}" ]; then
    echo "Emscripten requires Python 3.10 or newer." >&2
    exit 1
  fi

  if [ ! -d "$EMSDK_DIR" ]; then
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
  fi

  "$EMSDK_DIR/emsdk" install "$EMSDK_VERSION"
  "$EMSDK_DIR/emsdk" activate "$EMSDK_VERSION"
  # shellcheck disable=SC1091
  source "$EMSDK_DIR/emsdk_env.sh"
fi

rm -rf dist
mkdir -p dist

rm -rf "$WEB_ASSETS_DIR"
mkdir -p "$WEB_ASSETS_DIR"
find assets -maxdepth 1 -type f ! -name '*.mp3' -exec cp {} "$WEB_ASSETS_DIR" \;

SOURCES=(
  src/main.cpp
  src/Game.cpp
  src/ScreenHome.cpp
  src/ScreenLevelSelect.cpp
  src/ScreenLevel1.cpp
  src/ScreenLevel2.cpp
  src/ScreenLevel3.cpp
  src/Button.cpp
  src/ScreenNameInput.cpp
  src/ScreenHelp.cpp
  src/ScreenCredits.cpp
  src/ScreenHighestScore.cpp
)

em++ "${SOURCES[@]}" \
  -std=c++17 \
  -Iinclude \
  -O2 \
  -sUSE_SDL=2 \
  -sUSE_SDL_IMAGE=2 \
  -sSDL2_IMAGE_FORMATS='["png"]' \
  -sUSE_SDL_TTF=2 \
  -sUSE_SDL_MIXER=2 \
  -sSDL2_MIXER_FORMATS='["wav"]' \
  -sALLOW_MEMORY_GROWTH=1 \
  -sEXIT_RUNTIME=0 \
  --preload-file "$WEB_ASSETS_DIR"@/assets \
  --shell-file web/shell.html \
  -o dist/index.html
