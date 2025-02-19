cmake_minimum_required(VERSION 3.0)
project(raylib_game)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Emscripten specific settings
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASYNCIFY -s ASSERTIONS=1 -s WASM=1")
endif()

# Find raylib
find_package(raylib REQUIRED)

# Add your source files

# Link libraries
target_link_libraries(${PROJECT_NAME} raylib)

# # If using assets, add preload
# if (EMSCRIPTEN)
#     set_target_properties(raylib_game PROPERTIES
#         LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
#     )
# endif()