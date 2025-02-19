#!/bin/bash

# Clean previous build
rm -rf build dist
mkdir -p build dist

# Configure and build
cd build
emcmake cmake .. \
    -DPLATFORM=Web \
    -DCMAKE_BUILD_TYPE=Release

emmake make

# Copy to dist
cp raylib_game.html ../dist/index.html
cp raylib_game.js ../dist/
cp raylib_game.wasm ../dist/
cp raylib_game.data ../dist/

cd ..