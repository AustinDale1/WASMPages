cmake_minimum_required(VERSION 3.0)
project(raylib_game)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Emscripten specific settings
if(EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 --preload-file assets")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Find raylib
find_package(raylib REQUIRED)

# Add your source files
add_executable(${PROJECT_NAME} main.cpp)

# Link libraries
target_link_libraries(${PROJECT_NAME} raylib)
