#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "net_package.h"
#include "net_util.h"

struct Client {
  bool connected{false};
  const char* host;
  const char* port;
  int sockfd;

  Client(const char* host, const char* port) : host(host), port(port) {
  }

  void start() {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* servinfo;
    int getaddrinfo_result = getaddrinfo(host, port, &hints, &servinfo);
    if (getaddrinfo_result != 0) {
      fprintf(stderr, "Failed getting client host address info: %s\n", gai_strerror(getaddrinfo_result));
      exit(EXIT_FAILURE);
    }

    struct addrinfo* p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("failed establishing a client socket");
        continue;
      }

      if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("client could not connect");
        continue;
      }

      break;
    }

    if (p == NULL) {
      exit(EXIT_FAILURE);
      fprintf(stderr, "socket and/or connection has failed\n");
    }

    char addr_buf[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, get_incoming_addr((struct sockaddr*)p->ai_addr), addr_buf, sizeof(addr_buf));

    freeaddrinfo(servinfo);

    connected = true;
  }

  ~Client() {
    if (connected) {
      close(sockfd);
    }
  }

  void send_msg(NetPackage&& pack) {
    char buf[256];
    memcpy(buf, &pack, sizeof(NetPackage));
    send(sockfd, buf, sizeof(NetPackage), 0);
  }
};