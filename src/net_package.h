#pragma once

enum class NetPackageKind {
  Move,
  Shoot,
  Explode,
};

struct NetPackageMove {
  float x;
  float y;
  float angle;
  unsigned char wrm_index;
  bool dir;
};

struct NetPackageExplode {
  float x;
  float y;
  float radius;
  // TODO: keep in mind when scaling wrm number.
  char new_health[4];
};

struct NetPackageShoot {
  float x;
  float y;
  float angle;
  float force;
};

struct NetPackage {
  union {
    NetPackageMove move;
    NetPackageExplode explode;
    NetPackageShoot shoot;
  };

  NetPackageKind kind;
};

constexpr size_t NET_PACKAGE_BYTE_LEN = sizeof(NetPackage);
