#include <ctype.h>
#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "app.h"

int main(void) {
  SetTraceLogLevel(LOG_WARNING);

  App app{};
  app.loop();

  return EXIT_SUCCESS;
}
