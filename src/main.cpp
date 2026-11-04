#include <raylib.h>

int main() {
  InitWindow(800, 800, "mandelbrot");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();

    {
      ClearBackground(DARKGRAY);
      DrawText("Hello World", 100, 100, 20, WHITE);
    }

    EndDrawing();
  }

  CloseWindow();
}
