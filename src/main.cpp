#include <ctype.h>
#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "app.h"

int main(void) {
  SetTraceLogLevel(LOG_WARNING);
  std::srand(std::time(nullptr));

  App app{};
  app.loop();

  return EXIT_SUCCESS;
}
