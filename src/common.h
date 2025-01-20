#pragma once

#include <raylib.h>

#include <cstdlib>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define GRAVITY_DEC 0.98f
#define GRAVITY_INC 1.03f
#define GRAVITY_BOOSTER_ADJUSTMENT 0.1f

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

bool out_of_screen(Vector2 const &pos) {
  return pos.x < 0 || pos.y < 0 || pos.x >= SCREEN_WIDTH || pos.y >= SCREEN_HEIGHT;
}

struct Gravity {
  float value{0.0f};

  void update() {
    if (value < 0.0f) {
      value = value * GRAVITY_DEC + GRAVITY_BOOSTER_ADJUSTMENT;
    } else {
      value = value * GRAVITY_INC + GRAVITY_BOOSTER_ADJUSTMENT;
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
  SMOKE,
  BULLET_MISSED,
};

struct CommandFire {
  Vector2 pos;
  float angle;
  float force;
};

struct CommandExplosion {
  Vector2 pos;
  float radius;  // How far does the damage reach.
  float power;   // How much does 100% hurt.
};

struct Command {
  union {
    CommandFire fire;
    CommandExplosion explosion;
    Vector2 smoke_pos;
  };
  CommandKind kind;
};

struct Hittable {
  virtual bool is_hit(Vector2 &point) const;
};

Command make_fire_command(Vector2 pos, float angle, float force) {
  return Command{.fire = {pos, angle, force}, .kind = CommandKind::FIRE};
}

Command make_explosion_command(Vector2 pos, float range, float power) {
  return Command{.explosion = {pos, range, power}, .kind = CommandKind::EXPLOSION};
}

Command make_smoke_command(Vector2 pos) {
  return Command{.smoke_pos = pos, .kind = CommandKind::SMOKE};
}

Command make_bullet_missed_command() {
  return Command{.kind = CommandKind::BULLET_MISSED};
}

bool color_is_transparent(const Color &color) {
  return color.a == 0;
}

struct Smoke {
  Vector2 pos;
  int timer{60};

  Smoke(Vector2 pos) : pos(pos) {
  }

  void draw() const {
    DrawCircleV(pos, (float)timer / 6.0f, Color{0xff, 0xff, 0xff, 0x20});
  }

  void update() {
    timer--;
  }

  bool is_dead() const {
    return timer <= 0;
  }
};

struct Rumble {
  Vector2 pos;
  float vx;
  Gravity g;
  float rot;
  float rot_v;
  int timer;
  float size;

  void update() {
    g.update();

    pos.x += vx;
    pos.y += g.value;

    rot += rot_v;

    timer--;
  }

  void draw() const {
    DrawRectanglePro({pos.x, pos.y, size, size}, {size / 2.0f, size / 2.0f}, rot, GRAY);
  }

  bool is_dead() const {
    return timer <= 0;
  }
};

float randf(float max) {
  return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * max;
}

Rumble make_rumble(Vector2 pos, float zone) {
  float zone_half = zone / 2.0f;
  return Rumble{
      .pos = Vector2{pos.x + randf(zone_half) - zone_half, pos.y + randf(zone_half) - zone_half},
      .vx = randf(10.0f) - 5.0f,
      .g = Gravity{randf(10.0f) - 12.0f},
      .rot = 0.0f,
      .rot_v = randf(30.0f) - 15.0f,
      .timer = std::rand() % 100,
      .size = randf(10.0f) + 5.0f,
  };
}
