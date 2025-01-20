#pragma once

#include <raylib.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "bullet.h"
#include "common.h"
#include "wrm.h"

#define APP_RUMBLE_COUNT 60

struct App {
  std::vector<Wrm> wrms{};
  int active_worm = -1;
  std::vector<Bullet> bullets{};
  std::vector<Smoke> smokes{};
  std::vector<Rumble> rumbles{};
  Image foreground_image;
  Texture2D background_texture;

  App() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wrms");
    SetTargetFPS(120);

    background_texture = LoadTexture("./data/background_1.png");

    reset();
  }

  ~App() {
    UnloadImage(foreground_image);
    CloseWindow();
  }

  void reset() {
    bullets.clear();
    smokes.clear();
    rumbles.clear();

    wrms.clear();
    wrms.emplace_back(Vector2{100.0f, 100.0f}, true, Color{0xff, 0xcc, 0xcc, 0xff});
    wrms.emplace_back(Vector2{(float)SCREEN_WIDTH - 100.0f, 100.0f}, false, Color{0xcc, 0xcc, 0xff, 0xff});
    wrms.emplace_back(Vector2{300.0f, 0.0f}, true, Color{0xff, 0xcc, 0xcc, 0xff});
    wrms.emplace_back(Vector2{(float)SCREEN_WIDTH - 300.0f, 0.0f}, false, Color{0xcc, 0xcc, 0xff, 0xff});

    foreground_image = LoadImage("./data/foreground_1.png");

    active_worm = 0;
  }

  void loop() {
    while (!WindowShouldClose()) {
      Color *colors = LoadImageColors(foreground_image);

      update(colors);

      UnloadImageColors(colors);

      BeginDrawing();
      ClearBackground(RAYWHITE);

      DrawTexture(background_texture, 0, 0, WHITE);

      Texture2D foreground_texture = LoadTextureFromImage(foreground_image);
      DrawTexture(foreground_texture, 0, 0, WHITE);

      draw();

      DrawFPS(0, 0);

      EndDrawing();

      UnloadTexture(foreground_texture);
    }
  }

  void update(Color *colors) {
    if (IsKeyPressed(KEY_R)) {
      reset();
      return;
    }

    std::vector<Command> output_commands{};

    std::vector<Hittable *> hittables{};
    for (auto &wrm : wrms) {
      hittables.push_back(&wrm);
    }

    for (auto &bullet : bullets) {
      bullet.update(output_commands, colors, hittables);
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const auto &bullet) { return bullet.is_dead; }),
                  bullets.end());

    for (auto &smoke : smokes) {
      smoke.update();
    }
    smokes.erase(std::remove_if(smokes.begin(), smokes.end(), [](const auto &smoke) { return smoke.is_dead(); }),
                 smokes.end());

    for (auto &rumble : rumbles) {
      rumble.update();
    }
    rumbles.erase(std::remove_if(rumbles.begin(), rumbles.end(), [](const auto &smoke) { return smoke.is_dead(); }),
                  rumbles.end());

    for (int i = 0; i < (int)wrms.size(); i++) {
      bool has_control = i == active_worm;
      wrms[i].update(output_commands, colors, has_control);
    }

    for (Command command : output_commands) {
      if (command.kind == CommandKind::FIRE) {
        bullets.emplace_back(command.fire);
      } else if (command.kind == CommandKind::EXPLOSION) {
        ImageDrawCircle(&foreground_image, command.explosion.pos.x, command.explosion.pos.y, command.explosion.radius,
                        FAKE_TRANSPARENT_COLOR);
        ImageColorReplace(&foreground_image, FAKE_TRANSPARENT_COLOR, TRANSPARENT_COLOR);

        for (int i = 0; i < APP_RUMBLE_COUNT; i++) {
          rumbles.push_back(make_rumble(command.explosion.pos, command.explosion.radius));
        }

        active_worm = (active_worm + 1) % wrms.size();

        for (auto &wrm : wrms) {
          float explosion_distance = Vector2Distance(wrm.pos, command.explosion.pos);
          float damage{0.0f};
          if (wrm.is_hit(command.explosion.pos)) {
            damage = command.explosion.power;
          } else if (explosion_distance < command.explosion.radius) {
            float damage_percentage = (command.explosion.radius - explosion_distance) / command.explosion.radius;
            damage = command.explosion.power * damage_percentage;
          }
          wrm.life -= damage;
        }
      } else if (command.kind == CommandKind::SMOKE) {
        smokes.push_back(Smoke{command.smoke_pos});
      } else if (command.kind == CommandKind::BULLET_MISSED) {
        active_worm = (active_worm + 1) % wrms.size();
      } else {
        TraceLog(LOG_ERROR, "Invalid command");
      }
    }
  }

  void draw() const {
    for (auto &wrm : wrms) {
      wrm.draw();
    }

    for (auto &bullet : bullets) {
      bullet.draw();
    }

    for (auto &smoke : smokes) {
      smoke.draw();
    }

    for (auto &rumble : rumbles) {
      rumble.draw();
    }
  }
};
