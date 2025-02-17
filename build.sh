#!/bin/bash

# Create build directory
mkdir -p build
cd build

# Configure with CMake
emcmake cmake ..

# Build
emmake make

# Create dist directory
cd ..
mkdir -p dist
cp build/raylib_game.* dist/
cp build/raylib_game.wasm dist/
cp build/raylib_game.data dist/