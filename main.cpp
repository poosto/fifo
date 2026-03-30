#include "fifo.h"
#include <print>

int main() {
  FIFO<int, 2, FIFOPolicy::SPSC> fifo{1, 2};

  std::println("val={}", fifo.peek());

  return 0;
}
