#!/bin/bash

# Create build directory
mkdir -p build
cd build

# Configure with CMake
emcmake cmake .. \
    -DPLATFORM=Web \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864"

# Build
emmake make

# Create dist directory
cd ..
mkdir -p dist
cp build/raylib_game.html dist/index.html
cp build/raylib_game.js dist/
cp build/raylib_game.wasm dist/
cp build/raylib_game.data dist/