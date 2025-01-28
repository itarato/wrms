#pragma once

#include <deque>
#include <mutex>
#include <optional>
#include <string>

template <typename T>
struct SharedMsgQueue {
  std::deque<T> queue{};
  std::mutex mtx{};

  std::optional<T> pop() {
    std::lock_guard<std::mutex> guard(mtx);
    if (queue.empty()) {
      return {};
    } else {
      T pack = queue.front();
      queue.pop_front();
      return pack;
    }
  }

  void push(T &&pack) {
    std::lock_guard<std::mutex> guard(mtx);
    queue.push_back(std::move(pack));
  }
};
