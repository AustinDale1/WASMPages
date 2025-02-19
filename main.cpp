#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame(void)
{
    BeginDrawing();
        ClearBackground(RED);  // Changed to RED to be more visible
        DrawText("Testing!", 10, 10, 20, WHITE);
    EndDrawing();
}

int main(void)
{
    InitWindow(800, 600, "Test");

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    #else
        SetTargetFPS(60);
        while (!WindowShouldClose())
        {
            UpdateDrawFrame();
        }
    #endif

    CloseWindow();
    return 0;
}