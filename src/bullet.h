#pragma once

#include <raylib.h>

#include "common.h"

struct Bullet {
  Thrust thrust;
  bool is_dead{false};

  Bullet(CommandFire &command) : thrust(command.pos, command.angle, command.force) {
  }

  void draw() const {
    DrawCircleV(thrust.pos, 10.0f, ORANGE);
  }

  void update(std::vector<Command> &output_commands, Color *colors) {
    thrust.update();

    if (out_of_screen(thrust.pos)) {
      is_dead = true;
      return;
    }

    if (!color_is_transparent(colors[(int)thrust.pos.y * SCREEN_WIDTH + (int)thrust.pos.x])) {
      output_commands.push_back(make_explosion_command(thrust.pos));
      is_dead = true;
      return;
    }
  }
};
