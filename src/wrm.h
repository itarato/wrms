#pragma once

#include <raylib.h>
#include <raymath.h>

#include "common.h"

#define WRM_HSPEED 1
#define WRM_MOVE_LIFT_THRESHOLD -5
#define WRM_AIM_CROSS_DIST 100
#define WRM_AIM_SPEED 1
#define WRM_SHOOT_MAX_FORCE 20.0f
#define WRM_SHOOT_FORCE_INCREMENT 0.1f

struct Wrm {
  Vector2 pos;
  Vector2 frame;
  float aim_angle{0.0f};
  bool is_dir_right{true};
  float shoot_force{0.0f};

  Wrm(Vector2 pos, Vector2 frame) : pos(pos), frame(frame) {
  }

  void draw() const {
    // DrawRectangleV(pos, frame, BROWN);
    if (is_dir_right) {
      DrawTriangle(Vector2Add(pos, {0.0f, frame.y}), Vector2Add(pos, frame), Vector2Add(pos, {frame.x, 0.0f}), BROWN);
    } else {
      DrawTriangle(pos, Vector2Add(pos, {0.0f, frame.y}), Vector2Add(pos, frame), BROWN);
    }

    Vector2 fire_center = get_fire_center();
    Vector2 aim_pos{
        cosf(DEG2RAD * aim_angle) * WRM_AIM_CROSS_DIST + fire_center.x,
        sinf(DEG2RAD * aim_angle) * WRM_AIM_CROSS_DIST + fire_center.y,
    };
    DrawCircleV(aim_pos, 20.0f, MAGENTA);

    if (shoot_force > 0.0f) {
      DrawText(TextFormat("%03.2f", shoot_force), pos.x, pos.y + frame.y + 20.0f, 20, BLACK);
    }
  }

  Vector2 get_fire_center() const {
    if (is_dir_right) {
      return Vector2Add(pos, {frame.x, 0.0f});
    } else {
      return pos;
    }
  }

  void update(std::vector<Command> &output_commands, Color *colors) {
    update_movement(colors);
    update_aim();
    update_shoot(output_commands);
  }

  void update_shoot(std::vector<Command> &output_commands) {
    if (IsKeyDown(KEY_SPACE)) {
      shoot_force += WRM_SHOOT_FORCE_INCREMENT;
      if (shoot_force > WRM_SHOOT_MAX_FORCE) shoot_force = WRM_SHOOT_MAX_FORCE;
    } else if (shoot_force > 0.0f) {
      output_commands.push_back(make_fire_command(get_fire_center(), aim_angle, shoot_force));
      shoot_force = 0.0f;
    }
  }

  void update_aim() {
    if (IsKeyDown(KEY_UP)) {
      aim_angle += WRM_AIM_SPEED;
    }
    if (IsKeyDown(KEY_DOWN)) {
      aim_angle -= WRM_AIM_SPEED;
    }
  }

  void update_movement(Color *colors) {
    int old_pos_x = pos.x;
    if (IsKeyDown(KEY_LEFT)) {
      pos.x -= WRM_HSPEED;
      is_dir_right = false;
    }
    if (IsKeyDown(KEY_RIGHT)) {
      pos.x += WRM_HSPEED;
      is_dir_right = true;
    }

    int left_bottom_x = pos.x;
    int left_bottom_y = pos.y + frame.y;
    int left_floor_y = next_floor_y(colors, left_bottom_x, left_bottom_y);

    int right_bottom_x = pos.x + frame.x;
    int right_bottom_y = pos.y + frame.y;
    int right_floor_y = next_floor_y(colors, right_bottom_x, right_bottom_y);

    int floor_y = std::min(left_floor_y, right_floor_y);
    int new_pos_y = floor_y - frame.y + 1;
    int floor_y_diff = new_pos_y - pos.y;

    if (floor_y_diff >= WRM_MOVE_LIFT_THRESHOLD) {
      pos.y = floor_y - frame.y + 1;
    } else {
      pos.x = old_pos_x;
    }
  }

 private:
  int next_floor_y(Color *colors, int for_x, int for_y) {
    int y = for_y;

    if (ColorIsEqual(colors[y * SCREEN_WIDTH + for_x], FILLED_MASK_COLOR)) {
      for (; y >= 0; y--) {
        if (ColorIsEqual(colors[y * SCREEN_WIDTH + for_x], EMPTY_MASK_COLOR)) {
          break;
        }
      }
    } else {
      for (; y < SCREEN_HEIGHT; y++) {
        if (ColorIsEqual(colors[y * SCREEN_WIDTH + for_x], FILLED_MASK_COLOR)) {
          y--;
          break;
        }
      }
    }

    return y;
  }
};
