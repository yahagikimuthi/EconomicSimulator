#pragma once

#include <compare>

#include "core/base.hpp"

namespace internal {
template <typename T>
class [[nodiscard]] BaseValueClass {
  public:
    explicit BaseValueClass(T value) : value_{value} {}
    auto value() const -> T { return value_; }

  protected:
    auto operator<=>(const BaseValueClass<T>&) const = default;
    T    value_;
};
}  // namespace internal

class [[nodiscard]] Money : public internal::BaseValueClass<double> {
  public:
    using internal::BaseValueClass<double>::BaseValueClass;
    auto operator<=>(const Money&) const = default;
    auto operator+=(const Money other) -> Money& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const Money other) -> Money& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) -> Money& {
        value_ *= other;
        return *this;
    }
    auto operator/=(const double other) -> Money& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }
};
[[nodiscard]] inline auto operator+(Money lhs, Money rhs) -> Money {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(Money lhs, Money rhs) -> Money {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(Money lhs, double rhs) -> Money {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(double lhs, Money rhs) -> Money { return rhs * lhs; }
[[nodiscard]] inline auto operator/(Money lhs, double rhs) -> Money {
    lhs /= rhs;
    return lhs;
}

class [[nodiscard]] AgentID : internal::BaseValueClass<int> {
    AgentID(const int value) PRE(value >= 0)
        : internal::BaseValueClass<int>::BaseValueClass(value) {}
    auto operator<=>(const AgentID&) const = default;
};