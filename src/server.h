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
#include "shared_msg_queue.h"

struct Server {
  const char* port;
  int sockfd;
  SharedMsgQueue<NetPackage>* queue;
  std::atomic_bool* is_over;

  Server(const char* port, SharedMsgQueue<NetPackage>* queue, std::atomic_bool* is_over)
      : port(port), queue(queue), is_over(is_over) {
  }

  ~Server() {
    close(sockfd);
  }

  bool establish() {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* servinfo;
    int getaddrinfo_result = getaddrinfo(NULL, port, &hints, &servinfo);

    if (getaddrinfo_result != 0) {
      fprintf(stderr, "Failed getting address info\n");
      return false;
    }

    bool did_bind{false};
    for (struct addrinfo* p = servinfo; p != NULL; p = p->ai_next) {
      sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (sockfd == -1) {
        perror("socket cannot be established, trying the next one");
        continue;
      }

      if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        perror("FCNTL error");
        exit(EXIT_FAILURE);
      }

      int sock_opt_val = 1;
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt_val, sizeof(int)) == -1) {
        perror("cannot set socket options");
        return false;
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("cannot bind to socket");
        continue;
      }

      did_bind = true;
      break;
    }

    if (!did_bind) {
      fprintf(stderr, "failed to get socket and bind\n");
      exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 10) == -1) {
      perror("failed listening on socket");
      return false;
    }

    return true;
  }

  int run() {
    sockaddr_storage incoming_sockaddr;
    socklen_t sin_size = sizeof(incoming_sockaddr);
    int incoming_sockfd;
    char addr_buf[INET6_ADDRSTRLEN];
    char incoming_buf[1024];

    while (!is_over->load()) {
      incoming_sockfd = accept4(sockfd, (struct sockaddr*)&incoming_sockaddr, &sin_size, SOCK_NONBLOCK);
      if (incoming_sockfd == -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          perror("falled getting a socket for incoming call");
        }
        continue;
      }

      inet_ntop(incoming_sockaddr.ss_family, get_incoming_addr((struct sockaddr*)&incoming_sockaddr), addr_buf,
                sizeof(addr_buf));

      while (!is_over->load()) {
        int incoming_bytes_len = recv(incoming_sockfd, incoming_buf, 1024, MSG_DONTWAIT);
        if (incoming_bytes_len == -1) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
          }

          perror("failed reading incoming message");
          break;
        } else if (incoming_bytes_len == 0) {
          // EOF
          fprintf(stderr, "Connection ended.\n");
          break;
        } else {
          if (incoming_bytes_len != NET_PACKAGE_BYTE_LEN) {
            fprintf(stderr, "incoming message lenght is not net-package sized");
            continue;
          }

          NetPackage pack;
          memcpy(&pack, incoming_buf, incoming_bytes_len);
          queue->push(std::move(pack));

          printf("Incoming (%d) bytes\n", incoming_bytes_len);
        }
      }

      close(incoming_sockfd);
    }

    close(sockfd);

    return EXIT_SUCCESS;
  }
};
