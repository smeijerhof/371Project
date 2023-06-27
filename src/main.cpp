#include <../include/raylib.h>

int main () {
  InitWindow(400, 400, "window_name");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(10, 10);
    EndDrawing();
  }

  CloseWindow();
  
  return 0;
}
