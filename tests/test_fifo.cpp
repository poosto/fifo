#include "../fifo.h"
#include "catch_amalgamated.hpp"
#include <atomic>
#include <thread>

struct HeapObj {
  int *x_;
  constexpr HeapObj(int x) : x_{new int{x}} {}
  constexpr HeapObj(const HeapObj &other) : x_{new int{*other.x_}} {}
  constexpr ~HeapObj() { delete x_; };
  constexpr bool operator==(const HeapObj &other) const {
    return *x_ == *other.x_;
  }
};

TEST_CASE("empty construction runtime", "[fifo]") {
  FIFO<HeapObj, 5, FIFOPolicy::SPSC> fifo{};
  REQUIRE(fifo.size() == 0);
}

TEST_CASE("empty construction comptime", "[fifo][consteval]") {
  static_assert([]() consteval {
    FIFO<HeapObj, 3, FIFOPolicy::SPSC> fifo{};
    return fifo.size() == 0;
  }());
}

TEST_CASE("initializer list construction runtime", "[fifo]") {
  FIFO<HeapObj, 3, FIFOPolicy::SPSC> fifo{67, 68, 69};
  REQUIRE(fifo.size() == 3);
}

TEST_CASE("initializer list construction comptime", "[fifo][consteval]") {
  static_assert([]() consteval {
    FIFO<HeapObj, 3, FIFOPolicy::SPSC> fifo{67, 68, 69};
    return fifo.size() == 3;
  }());
}

TEST_CASE("copy construction runtime", "[fifo]") {
  FIFO<int, 3, FIFOPolicy::SPSC> fifo{0xA, 0xB, 0xC};
  FIFO<int, 3, FIFOPolicy::SPSC> copied{fifo};

  REQUIRE(fifo.size() == 3);
  REQUIRE(copied.size() == 3);

  for (auto it1 = fifo.begin(), it2 = copied.begin(); it1 != fifo.end();
       ++it1, ++it2) {
    REQUIRE(*it1 == *it2);
  }
}

TEST_CASE("copy construction comptime", "[fifo][consteval]") {
  static_assert([]() consteval {
    FIFO<HeapObj, 3, FIFOPolicy::SPSC> fifo{0xA, 0xB, 0xC};
    FIFO<HeapObj, 3, FIFOPolicy::SPSC> copied{fifo};

    for (auto it1 = fifo.begin(), it2 = copied.begin(); it1 != fifo.end();
         ++it1, ++it2) {
      if (*it1 != *it2) {
        return false;
      }
    }

    return fifo.size() == 3 && copied.size() == 3;
  }());
}

// TEST_CASE("concurrent push", "[fifo][threading]") {
//   FIFO<int, 100, FIFOPolicy::SPMC> fifo{};
//
//   auto worker = [&]() { fifo.push(67); };
//
//   std::thread t1{worker};
//   std::thread t2{worker};
//   t1.join();
//   t2.join();
// }
