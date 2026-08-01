#pragma once

#include <cassert>

#ifdef __clang__
#define PRE(...)
#define POST(...)
#define ASSERT(...) assert(__VA_ARGS__)
#else
#define PRE(...)
#define POST(...)
#define ASSERT(...) contract_assert(__VA_ARGS__)
#endif

template <typename T>
class [[nodiscard]] SafePtr {
  public:
    SafePtr(T* ptr) : ptr_{ptr} {}

    template <typename U>
    SafePtr(const SafePtr<U> other) : ptr_{other.get()} {}

    auto hasValue() const -> bool { return ptr_ != nullptr; }
    auto get() const -> T* { return ptr_; }

    auto operator*() const -> T& {
        assert(ptr_ != nullptr && "Attempted to access a null pointer via SafePtr");
        return *ptr_;
    }
    auto operator->() const -> T* {
        assert(ptr_ != nullptr && "Attempted to access a null pointer via SafePtr");
        return ptr_;
    }
    auto operator=(T* ptr) -> SafePtr& {
        ptr_ = ptr;
        return *this;
    }

    explicit operator bool() const { return ptr_ != nullptr; }

    SafePtr(const SafePtr&)                    = default;
    auto operator=(const SafePtr&) -> SafePtr& = default;
    SafePtr(SafePtr&&)                         = default;
    auto operator=(SafePtr&&) -> SafePtr&      = default;
    ~SafePtr()                                 = default;

  private:
    T* ptr_ = nullptr;
};