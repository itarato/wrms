#pragma once

#include <raylib.h>
#include <raymath.h>

#include "client.h"
#include "common.h"

#define WRM_HSPEED 0.4f
#define WRM_MOVE_LIFT_THRESHOLD -10
#define WRM_AIM_CROSS_DIST 100
#define WRM_AIM_SPEED 1
#define WRM_SHOOT_MAX_FORCE 20.0f
#define WRM_SHOOT_FORCE_INCREMENT 0.1f
#define WRM_JUMP_FORCE -5.0f
// How close it has to be to the platform to be considered on the ground;
#define WRM_ON_THE_GROUND_THRESHOLD 10.0f

struct Wrm : Hittable {
  unsigned char id;
  Vector2 pos;
  Vector2 frame;
  Gravity g{};
  float __aim_angle{135.0f};
  bool is_dir_right{true};
  float shoot_force{0.0f};
  Texture2D texture_left;
  Texture2D texture_right;
  Texture2D texture_aim;
  int life{100};
  Color tint;
  Client *client;

  Wrm(unsigned char id, Vector2 pos, bool is_dir_right, Color tint, Client *client)
      : id(id), pos(pos), is_dir_right(is_dir_right), tint(tint), client(client) {
    texture_left = LoadTexture("./data/ghost_left.png");
    texture_right = LoadTexture("./data/ghost_right.png");
    texture_aim = LoadTexture("./data/aim.png");
    frame = Vector2{(float)texture_left.width, (float)texture_left.height};
  }

  void draw(bool has_control) const {
    if (has_control) {
      Vector2 fire_center = get_fire_center();

      if (shoot_force > 0.0f) {
        float force_percentage = shoot_force / (float)WRM_SHOOT_MAX_FORCE;
        Vector2 force_end_pos{
            sinf(DEG2RAD * get_aim_angle()) * WRM_AIM_CROSS_DIST * force_percentage + fire_center.x,
            cosf(DEG2RAD * get_aim_angle()) * WRM_AIM_CROSS_DIST * force_percentage + fire_center.y,
        };
        DrawLineEx(get_fire_center(), force_end_pos, 10.0,
                   Color{u_int8_t(255.0f * shoot_force / WRM_SHOOT_MAX_FORCE), 0x00, 0x00, 0xff});
      }

      Vector2 aim_pos{
          sinf(DEG2RAD * get_aim_angle()) * WRM_AIM_CROSS_DIST + fire_center.x,
          cosf(DEG2RAD * get_aim_angle()) * WRM_AIM_CROSS_DIST + fire_center.y,
      };
      DrawTexturePro(texture_aim,
                     {
                         0.0f,
                         0.0f,
                         (float)texture_aim.width,
                         (float)texture_aim.height,
                     },
                     {
                         aim_pos.x,
                         aim_pos.y,
                         (float)texture_aim.width,
                         (float)texture_aim.height,
                     },
                     {

                         (float)texture_aim.width / 2,
                         (float)texture_aim.height / 2,

                     },
                     -get_aim_angle(), WHITE);
    }

    // Draw body.
    Vector2 texture_draw_pos = Vector2Subtract(pos, {frame.x / 2, frame.y});
    if (is_dir_right) {
      DrawTextureV(texture_right, texture_draw_pos, tint);
    } else {
      DrawTextureV(texture_left, texture_draw_pos, tint);
    }

    // Draw life.
    Rectangle life_rec{pos.x - 15.0f, pos.y - frame.y - 30.0f, 30.0f, 18.0f};
    DrawRectangleRec(life_rec, BLACK);
    DrawRectangleLinesEx(life_rec, 2, ORANGE);
    DrawText(TextFormat("%03d", life), pos.x - 8.0f, pos.y - frame.y - 25.0f, 10, ORANGE);
  }

  Vector2 get_fire_center() const {
    return Vector2Subtract(pos, {0.0f, frame.y / 2});
  }

  float get_aim_angle() const {
    if (is_dir_right) {
      return __aim_angle;
    } else {
      return 360.0f - __aim_angle;
    }
  }

  void update(std::vector<Command> &output_commands, Color *colors, bool has_control) {
    update_movement(colors, has_control);
    update_aim(has_control);
    update_shoot(output_commands, has_control);
  }

  void update_shoot(std::vector<Command> &output_commands, bool has_control) {
    if (has_control && IsKeyDown(KEY_SPACE)) {
      shoot_force += WRM_SHOOT_FORCE_INCREMENT;
      if (shoot_force > WRM_SHOOT_MAX_FORCE) shoot_force = WRM_SHOOT_MAX_FORCE;
    } else if (shoot_force > 0.0f) {
      output_commands.push_back(make_fire_command(get_fire_center(), get_aim_angle(), shoot_force));
      shoot_force = 0.0f;
    }
  }

  void update_aim(bool has_control) {
    if (has_control && IsKeyDown(KEY_UP)) {
      __aim_angle += WRM_AIM_SPEED;

      if (__aim_angle > 180.0f) __aim_angle = 180.0f;
    }
    if (has_control && IsKeyDown(KEY_DOWN)) {
      __aim_angle -= WRM_AIM_SPEED;
      if (__aim_angle < 0.0f) __aim_angle = 0.0f;
    }
  }

  void update_movement(Color *colors, bool has_control) {
    update_vertical_movement(colors, has_control);
    update_horizontal_movement(colors, has_control);

    if (client->connected) {
      client->send_msg(NetPackage{.move =
                                      {
                                          .x = pos.x,
                                          .y = pos.y,
                                          .angle = __aim_angle,
                                          .wrm_index = id,
                                          .dir = is_dir_right,
                                      },
                                  .kind = NetPackageKind::Move});
    }
  }

  void update_vertical_movement(Color *colors, bool has_control) {
    g.update();

    int floor_y = next_floor_y(colors, pos.x, pos.y);
    // TraceLog(LOG_WARNING, "Current Y: %f Next Y: %d", pos.y, floor_y);
    int floor_y_diff = floor_y - pos.y;
    bool is_on_the_ground = fabs(floor_y_diff) < WRM_ON_THE_GROUND_THRESHOLD;

    if (has_control && IsKeyPressed(KEY_ENTER) && is_on_the_ground) {
      g.value = WRM_JUMP_FORCE;
    }

    if (g.rise()) {  // Rising.
      int ceiling_y = next_ceiling_y(colors);

      if (ceiling_y < pos.y) {  // Can rise.
        int rise_y = pos.y + g.value;
        if (rise_y < ceiling_y) {  // Can rise partial amount only.
          pos.y = ceiling_y;
          g.reset();
        } else {
          pos.y = rise_y;
        }
      } else {  // Cannot rise.
        g.reset();
      }
    } else {                                         // Falling.
      if (floor_y_diff < WRM_MOVE_LIFT_THRESHOLD) {  // Got inside a wall - cannot move there.
        // This situation is awkward - we cannot triviall fix the position.
        // We need to avoid at all cost to move the wrm into the wall.
        g.reset();
      } else if (floor_y_diff <= 0.0f) {  // Lifting slightly up.
        pos.y = floor_y;
        g.reset();
      } else {  // Falling.
        int fallen_y = pos.y + g.value;
        if (fallen_y > floor_y) {  // Reached floor.
          pos.y = floor_y;
          g.bounce();
        } else {  // Free falling.
          pos.y = fallen_y;
        }
      }
    }
  }

  void update_horizontal_movement(Color *colors, bool has_control) {
    float new_pos_x{pos.x};
    bool has_moved{false};

    if (has_control && IsKeyDown(KEY_LEFT)) {
      new_pos_x = pos.x - WRM_HSPEED;
      is_dir_right = false;
      has_moved = true;
    }
    if (has_control && IsKeyDown(KEY_RIGHT)) {
      new_pos_x = pos.x + WRM_HSPEED;
      is_dir_right = true;
      has_moved = true;
    }

    if (has_moved) {
      if (color_is_transparent(colors[(int)pos.y * SCREEN_WIDTH + (int)new_pos_x])) {
        pos.x = new_pos_x;
      } else {
        int floor_y = next_floor_up_y(colors, new_pos_x, pos.y);
        int diff = floor_y - pos.y;
        if (diff >= WRM_MOVE_LIFT_THRESHOLD) {
          pos.x = new_pos_x;
          pos.y = floor_y;
        }
      }
    }
  }

  Rectangle get_frame_rec() const {
    return Rectangle{pos.x, pos.y, frame.x, frame.y};
  }

  bool is_hit(Vector2 &point) const {
    return CheckCollisionPointRec(point, get_frame_rec());
  }

 private:
  int next_floor_y(Color *colors, int for_x, int for_y) {
    int y = for_y;
    // Color c = colors[for_y * SCREEN_WIDTH + for_x];
    // TraceLog(LOG_WARNING, "Color at x=%d y=%d -> R=%d G=%d B=%d A=%d", for_x, for_y, c.r, c.g, c.b, c.a);

    if (color_is_transparent(colors[y * SCREEN_WIDTH + for_x])) {
      for (; y < SCREEN_HEIGHT; y++) {
        if (!color_is_transparent(colors[y * SCREEN_WIDTH + for_x])) {
          y--;
          break;
        }
      }
    } else {
      for (; y > 0; y--) {
        if (color_is_transparent(colors[y * SCREEN_WIDTH + for_x])) {
          break;
        }
      }
    }

    return bound_value(y, 0, SCREEN_HEIGHT - 1);
  }

  int next_floor_up_y(Color *colors, int for_x, int for_y) {
    int y = for_y;
    for (; y > 0; y--) {
      if (color_is_transparent(colors[y * SCREEN_WIDTH + for_x])) {
        break;
      }
    }

    return bound_value(y, 0, SCREEN_HEIGHT - 1);
  }

  int next_ceiling_y(Color *colors) {
    int y = pos.y;
    // Color c = colors[for_y * SCREEN_WIDTH + for_x];
    // TraceLog(LOG_WARNING, "Color at x=%d y=%d -> R=%d G=%d B=%d A=%d", for_x, for_y, c.r, c.g, c.b, c.a);

    if (color_is_transparent(colors[y * SCREEN_WIDTH + (int)pos.x])) {
      for (; y > 0; y--) {
        if (!color_is_transparent(colors[y * SCREEN_WIDTH + (int)pos.x])) {
          y++;
          break;
        }
      }
    } else {
      for (; y < SCREEN_HEIGHT; y++) {
        if (color_is_transparent(colors[y * SCREEN_WIDTH + (int)pos.x])) {
          break;
        }
      }
    }

    if (y < 0) {
      return 0;
    } else if (y >= SCREEN_HEIGHT) {
      return SCREEN_HEIGHT - 1;
    } else {
      return y;
    }
  }
};
