#include <ctype.h>
#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <vector>

#include "app.h"
#include "net_package.h"
#include "server.h"
#include "shared_msg_queue.h"

void server_thread_call(const char* port, SharedMsgQueue<NetPackage>* queue, std::atomic_bool* is_over) {
  Server server{port, queue, is_over};
  if (!server.establish()) {
    fprintf(stderr, "Failed establishing server\n");
    exit(EXIT_FAILURE);
  }
  server.run();
}

int main(int argc, char** argv) {
  SetTraceLogLevel(LOG_WARNING);
  std::srand(std::time(nullptr));

  if (argc != 4) {
    fprintf(stderr, "Missing args: SERVER_PORT CLIENT_HOST CLIENT_PORT\n");
    exit(EXIT_FAILURE);
  }

  const char* server_port = argv[1];
  const char* client_host = argv[2];
  const char* client_port = argv[3];

  std::atomic_bool is_over{false};

  SharedMsgQueue<NetPackage>* queue = new SharedMsgQueue<NetPackage>();
  std::thread server_thread(server_thread_call, server_port, queue, &is_over);

  App app{client_host, client_port, queue};
  app.loop();

  is_over.store(true);

  server_thread.join();

  return EXIT_SUCCESS;
}
