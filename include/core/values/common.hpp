#pragma once

#include <compare>

#include "core/assertion.hpp"

namespace abm {
class Money final {
  public:
    [[nodiscard]] explicit constexpr Money(const double value) noexcept : value_{value} {}
    [[nodiscard]] constexpr auto value() const noexcept -> double { return value_; }
    [[nodiscard]] constexpr auto operator<=>(const Money&) const noexcept -> auto = default;

    constexpr auto operator+=(const Money other) noexcept -> Money& {
        value_ += other.value();
        return *this;
    }
    constexpr auto operator-=(const Money other) noexcept -> Money& {
        value_ -= other.value();
        return *this;
    }
    constexpr auto operator*=(const double other) noexcept -> Money& {
        value_ *= other;
        return *this;
    }
    constexpr auto operator/=(const double other) noexcept -> Money& {
        ASSERT(other != 0.0);
        value_ /= other;
        return *this;
    }

    [[nodiscard]] constexpr auto operator-() const noexcept -> Money { return Money{-value_}; }

  private:
    double value_;
};
[[nodiscard]] constexpr auto operator+(Money lhs, Money rhs) noexcept -> Money {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator-(Money lhs, Money rhs) noexcept -> Money {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(Money lhs, double rhs) noexcept -> Money {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(double lhs, Money rhs) noexcept -> Money {
    return rhs * lhs;
}
[[nodiscard]] constexpr auto operator/(Money lhs, double rhs) noexcept -> Money {
    ASSERT(rhs != 0.0);
    lhs /= rhs;
    return lhs;
}

class AgentID final {
  public:
    [[nodiscard]] explicit constexpr AgentID(const int value) noexcept : value_{value} {
        ASSERT(value >= -1);
    }
    [[nodiscard]] constexpr auto operator<=>(const AgentID&) const noexcept -> auto = default;

  private:
    int value_;
};

class Step final {
  public:
    [[nodiscard]] explicit constexpr Step(const int value) noexcept : value_{value} {}
    [[nodiscard]] constexpr auto value() const noexcept -> int { return value_; }
    [[nodiscard]] constexpr auto operator<=>(const Step&) const noexcept -> auto = default;

    constexpr auto operator++() -> Step& {
        ++value_;
        return *this;
    }
    [[nodiscard]] constexpr auto operator%(const int other) const noexcept -> Step {
        return Step{value_ % other};
    }

  private:
    int value_;
};
}  // namespace abm