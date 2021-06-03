#ifndef LVH_TIMEOUT_H
#define LVH_TIMEOUT_H

#include <Arduino.h>

class Timeout {
  unsigned long end;
public:
  void start(unsigned duration_ms) {
    end = millis() + duration_ms;
  }
  bool expired() {
    unsigned diff = end - millis();
    constexpr unsigned mask = ~(~0U >> 1);
    return diff & mask;
  }

  Timeout() {}
  Timeout(unsigned duration_ms) {
    start(duration_ms);
  }
};

template <unsigned ms>
class Every {
  unsigned long next;
  public:
  operator bool() {
    constexpr unsigned long signbit = ~(~0UL >> 1);
    if ((next - millis()) & signbit) {
      next += ms;
      return true;
    }
    return false;
  }
};

#endif // LVH_TIMEOUT_H
