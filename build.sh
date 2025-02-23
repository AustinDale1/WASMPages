#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Compile with Emscripten
emcc main.cpp \
    -o build/raylib_game.html \
    -s USE_GLFW=3 \
    -s WASM=1 \
    -s ASYNCIFY \
    -s ASSERTIONS=1 \
    -lraylib

# Serve the files (optional)
cd build
python -m http.server 8080