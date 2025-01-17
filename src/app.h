#include <raylib.h>

#include <vector>

#include "common.h"
#include "wrm.h"

struct App {
  RenderTexture2D render_texture;
  std::vector<Wrm> wrms{};
  int active_worm = -1;

  App() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wrms");
    SetTargetFPS(120);

    render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    BeginTextureMode(render_texture);
    ClearBackground(EMPTY_MASK_COLOR);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2, FILLED_MASK_COLOR);
    EndTextureMode();

    wrms.emplace_back(Vector2{100.0f, 100.0f}, Vector2{60.0f, 40.0f});
    active_worm = 0;
  }

  ~App() {
    UnloadRenderTexture(render_texture);
    CloseWindow();
  }

  void loop() {
    while (!WindowShouldClose()) {
      Vector2 mouse_coord = GetMousePosition();

      Image image = LoadImageFromTexture(render_texture.texture);
      Color *colors = LoadImageColors(image);
      Color mouse_color = colors[SCREEN_WIDTH * (int)mouse_coord.y + (int)mouse_coord.x];

      update(colors);

      UnloadImageColors(colors);

      BeginDrawing();
      ClearBackground(RAYWHITE);

      DrawTexture(render_texture.texture, 0, 0, WHITE);

      draw();

      DrawFPS(0, 0);

      EndDrawing();
    }
  }

  void update(Color *colors) {
    if (active_worm >= 0) {
      wrms[active_worm].update(colors);
    }

    if (IsMouseButtonPressed(0)) {
      Vector2 mouse_coord = GetMousePosition();
      BeginTextureMode(render_texture);
      DrawCircle(mouse_coord.x, SCREEN_HEIGHT - mouse_coord.y, 100.0f, EMPTY_MASK_COLOR);
      EndTextureMode();
    }
  }

  void draw() const {
    for (auto wrm : wrms) {
      wrm.draw();
    }
  }
};
