#pragma once

#include <raylib.h>

#include "common.h"

struct Bullet {
  float vx;
  Vector2 pos;
  Gravity g;
  bool is_dead{false};

  Bullet(CommandFire &command) {
    vx = sinf(DEG2RAD * command.angle) * command.force;
    pos = command.pos;
    g = Gravity{cosf(DEG2RAD * command.angle) * command.force};
  }

  void draw() const {
    DrawCircleV(pos, 10.0f, ORANGE);
  }

  void update(std::vector<Command> &output_commands, Color *colors) {
    g.update();

    pos.x += vx;
    pos.y += g.value;

    if (out_of_screen(pos)) {
      is_dead = true;
      return;
    }

    if (!color_is_transparent(colors[(int)pos.y * SCREEN_WIDTH + (int)pos.x])) {
      output_commands.push_back(make_explosion_command(pos));
      is_dead = true;
      return;
    }
  }
};
