#pragma once

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FILLED_MASK_COLOR DARKPURPLE
#define EMPTY_MASK_COLOR RAYWHITE

#define GRAVITY_DEC 0.95f
#define GRAVITY_INC 1.04f
#define GRAVITY_FALL_THRESHOLD 1.0f

enum class CommandKind {
  FIRE,
};

struct CommandFire {
  Vector2 pos;
  float angle;
  float force;
};

struct Command {
  union {
    CommandFire fire;
  };
  CommandKind kind;
};

Command make_fire_command(Vector2 pos, float angle, float force) {
  return Command{.fire = {pos, angle, force}, .kind = CommandKind::FIRE};
}
