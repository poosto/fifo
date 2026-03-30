#include <cstdlib>
#include <initializer_list>
#include <memory>

enum class FIFOPolicy {
  SPSC,
  SPMC,
  // etc...
};

template <typename T, size_t Capacity, FIFOPolicy Policy> class FIFO;

template <typename T, size_t Capacity>
class FIFO<T, Capacity, FIFOPolicy::SPSC> {
  union Slot;

public:
  using value_type = T;
  using pointer_type = T *;
  using reference_type = T &;
  using const_reference_type = const T &;
  using size_type = size_t;

  struct iterator {
    constexpr iterator(Slot *ptr, size_t idx, size_t max_idx)
        : ptr_{ptr}, idx_{idx}, max_idx_{max_idx} {}
    constexpr ~iterator() = default;

    constexpr iterator &operator++() {
      if (idx_ < max_idx_) {
        ++idx_;
      }

      return *this;
    }

    constexpr bool operator==(const iterator &other) const {
      return idx_ == other.idx_;
    }

    constexpr bool operator!=(const iterator &other) const {
      return idx_ != other.idx_;
    }

    constexpr const_reference_type operator*() const {
      return ptr_[idx_].value;
    }

    Slot *ptr_{};
    size_t idx_{};
    const size_t max_idx_;
  };

  constexpr FIFO(std::initializer_list<T> list) : size_{list.size()} {
    size_t i = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
      std::construct_at(std::addressof(data_[i++].value), *it);
    }
  }

  constexpr FIFO(const FIFO &other) : size_{other.size()} {
    for (size_t i = 0; i < size_; ++i) {
      std::construct_at(std::addressof(data_[i].value), other.data_[i].value);
    }
  }

  constexpr ~FIFO() {
    for (size_t i = 0; i < size_; ++i) {
      std::destroy_at(std::addressof(data_[i].value));
    }
  }
  constexpr void push(const T &val) {
    // TODO: throw/assert/something?
    if (size_ >= Capacity)
      return;

    std::construct_at(std::addressof(data_[size_].value), val);
    ++size_;
  }

  constexpr size_t size() const { return size_; }
  consteval size_t capacity() const { return Capacity; }
  constexpr const_reference_type peek() const { return data_[size_ - 1].value; }

  constexpr iterator begin() { return iterator{data_, 0, size_}; }
  constexpr iterator end() { return iterator{data_, size_, size_}; }

private:
  // Can't use std::array since this holds actual T objects
  // std::array<std::byte, sizeof(T) * Capacity> data_;

  // Remember that trivial types are left indeterminate if not explicitly
  // default constructed. This is unlike non-trivial types with defined
  // constructors. So adding {} here zeros the buffer for no reason!
  // alignas(T) std::byte data_[sizeof(T) * Capacity];

  // Crazy union trick that avoids default initialization of non-trivial members
  // Unions avoid default construction of internal values
  // Also, you can't do one anonymous union with T[Capacity] inside,
  // because constexpr constructors track the creation of unions,
  // and the entire array must be default constructed for it to be considered
  // 'active'. So if you try to set data_[i], you're accessing a non-active
  // union! errors like: construction of subobject of member 'test_' of union
  // with no active member is not allowed in a constant expression.
  // By having an array of unions, each entry of the array has its own state
  // and you can activate the entire union by
  // "placement-new/std::construct_at"-ing each element.

  union Slot {
    T value;

    constexpr Slot() {}
    constexpr ~Slot() {}
  };

  Slot data_[Capacity];

  size_t size_{};
};
