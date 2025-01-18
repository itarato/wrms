#pragma once

#include <raylib.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "bullet.h"
#include "common.h"
#include "wrm.h"

struct App {
  RenderTexture2D render_texture;
  std::vector<Wrm> wrms{};
  int active_worm = -1;
  std::vector<Bullet> bullets{};

  App() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wrms");
    SetTargetFPS(120);

    render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    Image background_image = LoadImage("./data/background_1.png");
    Color *background_colors = LoadImageColors(background_image);

    BeginTextureMode(render_texture);
    ClearBackground(TRANSPARENT_COLOR);
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        if (background_colors[y * SCREEN_WIDTH + x].a > 0) {
          DrawPixel(x, SCREEN_HEIGHT - y - 1, background_colors[y * SCREEN_WIDTH + x]);
        }
      }
    }
    EndTextureMode();

    UnloadImageColors(background_colors);
    UnloadImage(background_image);

    wrms.emplace_back(Vector2{100.0f, 100.0f}, Vector2{60.0f, 40.0f});
    active_worm = 0;
  }

  ~App() {
    UnloadRenderTexture(render_texture);
    CloseWindow();
  }

  void loop() {
    while (!WindowShouldClose()) {
      Image image = LoadImageFromTexture(render_texture.texture);
      Color *colors = LoadImageColors(image);

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
    std::vector<Command> output_commands{};

    for (auto &bullet : bullets) {
      bullet.update(output_commands, colors);
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const auto &bullet) { return bullet.is_dead; }),
                  bullets.end());

    if (active_worm >= 0) {
      wrms[active_worm].update(output_commands, colors);
    }

    if (IsMouseButtonPressed(0)) {
      Vector2 mouse_coord = GetMousePosition();
      BeginTextureMode(render_texture);
      DrawCircle(mouse_coord.x, SCREEN_HEIGHT - mouse_coord.y, 100.0f, TRANSPARENT_COLOR);
      EndTextureMode();
    }

    for (Command command : output_commands) {
      if (command.kind == CommandKind::FIRE) {
        bullets.emplace_back(command.fire);
      } else if (command.kind == CommandKind::EXPLOSION) {
        BeginTextureMode(render_texture);
        DrawCircle(command.explosion.pos.x, SCREEN_HEIGHT - command.explosion.pos.y, 60.0f, TRANSPARENT_COLOR);
        EndTextureMode();
      } else {
        TraceLog(LOG_ERROR, "Invalid command");
      }
    }
  }

  void draw() const {
    for (auto wrm : wrms) {
      wrm.draw();
    }

    for (auto bullet : bullets) {
      bullet.draw();
    }
  }
};
