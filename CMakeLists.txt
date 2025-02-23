cmake_minimum_required(VERSION 3.0)
project(raylib_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASYNCIFY -s ASSERTIONS=1 -s WASM=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] --shell-file ${CMAKE_SOURCE_DIR}/shell.html")
    
    # Add these lines
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s WASM=1 -s USE_GLFW=3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s USE_GLFW=3")
endif()

find_package(raylib REQUIRED)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} raylib)