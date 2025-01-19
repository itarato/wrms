#pragma once

#include <raylib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define GRAVITY_DEC 0.96f
#define GRAVITY_INC 1.04f
#define GRAVITY_FALL_THRESHOLD 0.7f

const Color FAKE_TRANSPARENT_COLOR = MAGENTA;
const Color TRANSPARENT_COLOR = Color{0x0, 0x0, 0x0, 0x00};

bool between(float value, float lhs, float rhs) {
  return value >= lhs && value <= rhs;
}

int bound_value(int value, int min_bound, int max_bound) {
  if (value < min_bound) {
    return min_bound;
  } else if (value > max_bound) {
    return max_bound;
  } else {
    return value;
  }
}

float min_bound_value(float value, float min_bound) {
  if (value < min_bound) {
    return min_bound;
  } else {
    return value;
  }
}

bool out_of_screen(Vector2 &pos) {
  return pos.x < 0 || pos.y < 0 || pos.x >= SCREEN_WIDTH || pos.y >= SCREEN_HEIGHT;
}

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