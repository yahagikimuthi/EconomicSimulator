#pragma once

#include <compare>

#include "core/base.hpp"

namespace abm {
class Money {
  public:
    [[nodiscard]] explicit Money(const double value) : value_{value} {}
    [[nodiscard]] auto value() const -> double { return value_; }
    [[nodiscard]] auto operator<=>(const Money&) const = default;
    auto               operator+=(const Money other) -> Money& {
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

class AgentID {
  public:
    [[nodiscard]] explicit AgentID(const int value) PRE(value >= -1) : value_{value} {}
    [[nodiscard]] auto operator<=>(const AgentID&) const = default;

  private:
    int value_;
};

class Step {
  public:
    [[nodiscard]] explicit Step(const int value) : value_{value} {}
    [[nodiscard]] auto value() const -> int { return value_; }
    [[nodiscard]] auto operator<=>(const Step&) const = default;
    auto               operator++() -> Step& {
        ++value_;
        return *this;
    }
    [[nodiscard]] auto operator%(const int other) const -> Step { return Step{value_ % other}; }

  private:
    int value_;
};
}  // namespace abm