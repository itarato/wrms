#pragma once

#include <raylib.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "bullet.h"
#include "client.h"
#include "common.h"
#include "net_package.h"
#include "shared_msg_queue.h"
#include "wrm.h"

#define APP_RUMBLE_COUNT 60

struct App {
  std::vector<Wrm> wrms{};
  int active_worm = -1;
  std::vector<Bullet> bullets{};
  std::vector<Smoke> smokes{};
  std::vector<Rumble> rumbles{};
  Image foreground_image;
  Color *foreground_colors;
  Texture2D background_texture;
  Texture2D foreground_texture;
  Client client;
  SharedMsgQueue<NetPackage> *msg_queue;
  int team_mod_selector;

  App(const char *client_host, const char *client_port, SharedMsgQueue<NetPackage> *msg_queue)
      : client(client_host, client_port), msg_queue(msg_queue) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wrms");
    SetTargetFPS(120);

    background_texture = LoadTexture("./data/background_1.png");

    team_mod_selector = atoi(client_port) % 2;

    reset();
  }

  ~App() {
    UnloadImage(foreground_image);

    UnloadImageColors(foreground_colors);
    UnloadTexture(foreground_texture);

    CloseWindow();
  }

  void reset() {
    bullets.clear();
    smokes.clear();
    rumbles.clear();

    wrms.clear();
    wrms.emplace_back(0, Vector2{100.0f, 100.0f}, true, Color{0xff, 0xcc, 0xcc, 0xff}, &client);
    wrms.emplace_back(1, Vector2{(float)SCREEN_WIDTH - 100.0f, 100.0f}, false, Color{0xcc, 0xcc, 0xff, 0xff}, &client);
    wrms.emplace_back(2, Vector2{300.0f, 0.0f}, true, Color{0xff, 0xcc, 0xcc, 0xff}, &client);
    wrms.emplace_back(3, Vector2{(float)SCREEN_WIDTH - 300.0f, 0.0f}, false, Color{0xcc, 0xcc, 0xff, 0xff}, &client);

    foreground_image = LoadImage("./data/foreground_1.png");
    foreground_texture = LoadTextureFromImage(foreground_image);
    foreground_colors = LoadImageColors(foreground_image);

    active_worm = 0;
  }

  void loop() {
    while (!WindowShouldClose()) {
      update(foreground_colors);

      BeginDrawing();
      ClearBackground(RAYWHITE);

      DrawTexture(background_texture, 0, 0, WHITE);
      DrawTexture(foreground_texture, 0, 0, WHITE);

      draw();

      DrawFPS(10, 10);

      EndDrawing();
    }
  }

  void update(Color *colors) {
    if (IsKeyPressed(KEY_R)) {
      reset();
      return;
    }

    if (IsKeyPressed(KEY_C) && !client.connected) {
      client.start();
      TraceLog(LOG_INFO, "Client started");
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
      bool has_control = (i == active_worm) && (team_mod_selector == i % 2);
      wrms[i].update(output_commands, colors, has_control);
    }

    for (Command command : output_commands) {
      if (command.kind == CommandKind::FIRE) {
        bullets.emplace_back(command.fire.pos, command.fire.angle, command.fire.force, false);

        if (client.connected) {
          client.send_msg(NetPackage{
              .shoot =
                  {
                      .x = command.fire.pos.x,
                      .y = command.fire.pos.y,
                      .angle = command.fire.angle,
                      .force = command.fire.force,
                  },
              .kind = NetPackageKind::Shoot,
          });
        }
      } else if (command.kind == CommandKind::EXPLOSION) {
        register_explosion_on_map(command.explosion.pos, command.explosion.radius);

        set_next_wrm_active();

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

        if (client.connected) {
          client.send_msg(NetPackage{
              .explode =
                  {
                      .x = command.explosion.pos.x,
                      .y = command.explosion.pos.y,
                      .radius = command.explosion.radius,
                      .new_health = {(char)wrms[0].life, (char)wrms[1].life, (char)wrms[2].life, (char)wrms[3].life},
                  },
              .kind = NetPackageKind::Explode,
          });
        }
      } else if (command.kind == CommandKind::SMOKE) {
        smokes.push_back(Smoke{command.smoke_pos});
      } else if (command.kind == CommandKind::BULLET_MISSED) {
        set_next_wrm_active();

        if (client.connected) {
          client.send_msg(NetPackage{
              .explode =
                  {
                      .x = command.explosion.pos.x,
                      .y = command.explosion.pos.y,
                      .radius = 0.0f,
                      .new_health = {(char)wrms[0].life, (char)wrms[1].life, (char)wrms[2].life, (char)wrms[3].life},
                  },
              .kind = NetPackageKind::Explode,
          });
        }
      } else {
        TraceLog(LOG_ERROR, "Invalid command");
      }
    }

    update_from_network_messages();
  }

  void draw() const {
    for (int i = 0; i < (int)wrms.size(); i++) {
      wrms[i].draw(i == active_worm);
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

    const char *connected_str;
    if (client.connected) {
      connected_str = "connected";
    } else {
      connected_str = "not connected";
    }
    DrawText(TextFormat("Other host: %s port: %s (%s) | Player #%d | Current wrm: %d", client.host, client.port,
                        connected_str, team_mod_selector, active_worm),
             10, GetScreenHeight() - 30, 20, WHITE);
  }

  void register_explosion_on_map(Vector2 pos, float r) {
    if (r == 0.0f) return;

    ImageDrawCircle(&foreground_image, pos.x, pos.y, r, FAKE_TRANSPARENT_COLOR);
    ImageColorReplace(&foreground_image, FAKE_TRANSPARENT_COLOR, TRANSPARENT_COLOR);

    if (foreground_colors) {
      UnloadImageColors(foreground_colors);
    }
    foreground_colors = LoadImageColors(foreground_image);

    UnloadTexture(foreground_texture);
    foreground_texture = LoadTextureFromImage(foreground_image);

    for (int i = 0; i < APP_RUMBLE_COUNT; i++) {
      rumbles.push_back(make_rumble(pos, r));
    }
  }

  void update_from_network_messages() {
    while (true) {
      auto pack_result = msg_queue->pop();
      if (pack_result.has_value()) {
        apply_net_package(std::move(pack_result.value()));
      } else {
        break;
      }
    }
  }

  void set_next_wrm_active() {
    active_worm = (active_worm + 1) % wrms.size();
  }

  void apply_net_package(NetPackage &&pack) {
    if (pack.kind == NetPackageKind::Move) {
      wrms[pack.move.wrm_index].pos.x = pack.move.x;
      wrms[pack.move.wrm_index].pos.y = pack.move.y;
      wrms[pack.move.wrm_index].__aim_angle = pack.move.angle;
      wrms[pack.move.wrm_index].is_dir_right = pack.move.dir;
    } else if (pack.kind == NetPackageKind::Explode) {
      register_explosion_on_map({pack.explode.x, pack.explode.y}, pack.explode.radius);
      wrms[0].life = (int)pack.explode.new_health[0];
      wrms[1].life = (int)pack.explode.new_health[1];
      wrms[2].life = (int)pack.explode.new_health[2];
      wrms[3].life = (int)pack.explode.new_health[3];

      // Kill network owned rocket.
      bullets.erase(
          std::remove_if(bullets.begin(), bullets.end(), [](const auto &bullet) { return bullet.network_operated; }),
          bullets.end());

      set_next_wrm_active();
    } else if (pack.kind == NetPackageKind::Shoot) {
      bullets.emplace_back(Vector2{pack.shoot.x, pack.shoot.y}, pack.shoot.angle, pack.shoot.force, true);
    } else {
      TraceLog(LOG_ERROR, "Unhandled net package kind");
    }
  }
};
