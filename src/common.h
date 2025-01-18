#pragma once

#include <raylib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define GRAVITY_DEC 0.95f
#define GRAVITY_INC 1.04f
#define GRAVITY_FALL_THRESHOLD 1.0f

const Color FAKE_TRANSPARENT_COLOR = MAGENTA;
const Color TRANSPARENT_COLOR = Color{0xff, 0xff, 0xff, 0x00};
const Color MASK_COLOR_EMPTY = YELLOW;
const Color MASK_COLOR_FILLED = BLACK;

enum class CommandKind {
  FIRE,
  EXPLOSION,
};

struct CommandFire {
  Vector2 pos;
  float angle;
  float force;
};

struct CommandExplosion {
  Vector2 pos;
};

struct Command {
  union {
    CommandFire fire;
    CommandExplosion explosion;
  };
  CommandKind kind;
};

Command make_fire_command(Vector2 pos, float angle, float force) {
  return Command{.fire = {pos, angle, force}, .kind = CommandKind::FIRE};
}

Command make_explosion_command(Vector2 pos) {
  return Command{.explosion = {pos}, .kind = CommandKind::EXPLOSION};
}
