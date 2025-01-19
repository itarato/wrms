#pragma once

#include <raylib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define GRAVITY_DEC 0.96f
#define GRAVITY_INC 1.04f
#define GRAVITY_FALL_THRESHOLD 0.7f

const Color FAKE_TRANSPARENT_COLOR = MAGENTA;
const Color TRANSPARENT_COLOR = Color{0x0, 0x0, 0x0, 0x00};

struct Gravity {
  float value{0.0f};

  void update() {
    if (fabs(value) < GRAVITY_FALL_THRESHOLD) {
      value = GRAVITY_FALL_THRESHOLD;
    } else if (value < 0.0f) {
      value *= GRAVITY_DEC;
    } else {
      value *= GRAVITY_INC;
    }
  }

  void reset() {
    value = 0.0f;
  }

  bool rise() const {
    return value < 0.0f;
  }
};

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

bool color_is_transparent(const Color &color) {
  return color.a == 0;
}
