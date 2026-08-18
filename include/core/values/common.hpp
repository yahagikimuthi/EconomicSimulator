#pragma once

#include <compare>

#include "core/base.hpp"

namespace abm {
class Money {
  public:
    [[nodiscard]] explicit Money(const double value) noexcept : value_{value} {}
    [[nodiscard]] auto value() const noexcept -> double { return value_; }
    [[nodiscard]] auto operator<=>(const Money&) const noexcept -> auto = default;
    auto               operator+=(const Money other) noexcept -> Money& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const Money other) noexcept -> Money& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) noexcept -> Money& {
        value_ *= other;
        return *this;
    }
    auto operator/=(const double other) noexcept -> Money& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }

    [[nodiscard]] auto operator-() const noexcept -> Money { return Money{-value_}; }

  private:
    double value_;
};
[[nodiscard]] inline auto operator+(Money lhs, Money rhs) noexcept -> Money {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(Money lhs, Money rhs) noexcept -> Money {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(Money lhs, double rhs) noexcept -> Money {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(double lhs, Money rhs) noexcept -> Money { return rhs * lhs; }
[[nodiscard]] inline auto operator/(Money lhs, double rhs) noexcept -> Money {
    lhs /= rhs;
    return lhs;
}

class AgentID {
  public:
    [[nodiscard]] explicit AgentID(const int value) noexcept : value_{value} {
        ASSERT(value >= -1);
    }
    [[nodiscard]] auto operator<=>(const AgentID&) const noexcept -> auto = default;

  private:
    int value_;
};

class Step {
  public:
    [[nodiscard]] explicit Step(const int value) noexcept : value_{value} {}
    [[nodiscard]] auto value() const noexcept -> int { return value_; }
    [[nodiscard]] auto operator<=>(const Step&) const noexcept -> auto = default;
    auto               operator++() -> Step& {
        ++value_;
        return *this;
    }
    [[nodiscard]] auto operator%(const int other) const noexcept -> Step {
        return Step{value_ % other};
    }

  private:
    int value_;
};
}  // namespace abm