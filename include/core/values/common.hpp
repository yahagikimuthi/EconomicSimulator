#pragma once

#include <compare>

#include "core/base.hpp"

namespace abm {

class [[nodiscard]] Money {
  public:
    explicit Money(const double value) : value_{value} {}
    auto value() const -> double { return value_; }
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
    auto operator-() const -> Money { return Money{-value_}; }

  private:
    double value_;
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

class [[nodiscard]] AgentID {
  public:
    explicit AgentID(const int value) PRE(value >= -1) : value_{value} {}
    auto operator<=>(const AgentID&) const = default;

  private:
    int value_;
};

class [[nodiscard]] Step {
  public:
    explicit Step(const int value) : value_{value} {}
    auto value() const -> int { return value_; }
    auto operator<=>(const Step&) const = default;
    auto operator++() -> Step& {
        ++value_;
        return *this;
    }
    auto operator%(const int other) const -> Step { return Step{value_ % other}; }

  private:
    int value_;
};
}  // namespace abm