#pragma once

#include <raylib.h>

#include "common.h"

struct Bullet {
  Vector2 pos;
  Vector2 v;
  bool is_dead{false};

  Bullet(Vector2 pos, Vector2 v) : pos(pos), v(v) {
  }

  Bullet(CommandFire &command) {
    pos = command.pos;
    v = Vector2{sinf(command.angle * DEG2RAD) * command.force, cosf(command.angle * DEG2RAD) * command.force};
  }

  void draw() const {
    DrawCircleV(pos, 10.0f, BLUE);
  }

  void update(std::vector<Command> &output_commands, Color *colors) {
    if (fabs(v.y) < GRAVITY_FALL_THRESHOLD) {
      v.y = GRAVITY_FALL_THRESHOLD;
    } else if (v.y < 0.0f) {
      v.y *= GRAVITY_DEC;
    } else {
      v.y *= GRAVITY_INC;
    }

    pos.x += v.x;
    pos.y += v.y;

    if (pos.x < 0 || pos.y < 0 || pos.x >= SCREEN_WIDTH || pos.y >= SCREEN_HEIGHT) {
      is_dead = true;
      return;
    }

    if (ColorIsEqual(colors[(int)pos.y * SCREEN_WIDTH + (int)pos.x], MASK_COLOR_FILLED)) {
      output_commands.push_back(make_explosion_command(pos));
      is_dead = true;
      return;
    }
  }
};
