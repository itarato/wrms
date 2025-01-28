#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

void* get_incoming_addr(struct sockaddr* sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  } else {
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
}
