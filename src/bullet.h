#pragma once

#include <raylib.h>

#include "common.h"

#define BULLET_EXPLOSION_ZONE_RADIUS 60.0f
#define BULLET_EXPLOSION_POWER 50.0f

struct Bullet {
  float vx;
  Vector2 pos;
  Gravity g;
  bool is_dead{false};
  bool network_operated;

  Bullet(Vector2 pos, float angle, float force, bool network_operated) : pos(pos), network_operated(network_operated) {
    vx = sinf(DEG2RAD * angle) * force;
    g = Gravity{cosf(DEG2RAD * angle) * force};
  }

  void draw() const {
    DrawCircleV(pos, 10.0f, ORANGE);
  }

  void update(std::vector<Command> &output_commands, Color *colors, std::vector<Hittable *> hittables) {
    g.update();

    pos.x += vx;
    pos.y += g.value;

    output_commands.push_back(make_smoke_command(pos));

    if (!network_operated) {
      // If out of screen.
      if (out_of_screen_sides_or_down(pos)) {
        output_commands.push_back(make_bullet_missed_command());
        is_dead = true;
        return;
      }

      // If hitting a wrm.
      for (auto hittable : hittables) {
        if (hittable->is_hit(pos)) {
          output_commands.push_back(make_explosion_command(pos, BULLET_EXPLOSION_ZONE_RADIUS, BULLET_EXPLOSION_POWER));
          is_dead = true;
          return;
        }
      }

      // If hitting ground.
      if (pos.y >= 0 && !color_is_transparent(colors[(int)pos.y * SCREEN_WIDTH + (int)pos.x])) {
        output_commands.push_back(make_explosion_command(pos, BULLET_EXPLOSION_ZONE_RADIUS, BULLET_EXPLOSION_POWER));
        is_dead = true;
        return;
      }
    }
  }
};
