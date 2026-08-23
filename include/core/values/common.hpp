#pragma once

#include <compare>

#include "core/assertion.hpp"

namespace abm::value_object {
template <typename Derived>
class ValueObjectMixin {
    friend Derived;

  public:
    [[nodiscard]] constexpr auto value() const noexcept -> double { return value_; }

    [[nodiscard]] friend constexpr auto operator<=>(Derived lhs, Derived rhs) noexcept -> auto {
        return lhs.value_ <=> rhs.value_;
    }
    [[nodiscard]] friend constexpr auto operator==(Derived lhs, Derived rhs) noexcept -> bool {
        return lhs.value_ == rhs.value_;
    }
    friend constexpr auto operator+=(Derived& lhs, Derived rhs) noexcept -> Derived& {
        lhs.value_ += rhs.value_;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator+(Derived lhs, Derived rhs) noexcept -> Derived {
        lhs += rhs;
        return lhs;
    }
    friend constexpr auto operator-=(Derived& lhs, Derived rhs) noexcept -> Derived& {
        lhs.value_ -= rhs.value_;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator-(Derived lhs, Derived rhs) noexcept -> Derived {
        lhs -= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator/(Derived lhs, Derived rhs) noexcept -> double {
        ASSERT(rhs.value_ != 0.0);
        return lhs.value_ / rhs.value_;
    }
    friend constexpr auto operator*=(Derived& lhs, double rhs) noexcept -> Derived& {
        lhs.value_ *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(Derived lhs, double rhs) noexcept -> Derived {
        lhs *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(double lhs, Derived rhs) noexcept -> Derived {
        return rhs * lhs;
    }
    friend constexpr auto operator/=(Derived& lhs, double rhs) noexcept -> Derived& {
        ASSERT(rhs != 0.0);
        lhs.value_ /= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator/(Derived lhs, double rhs) noexcept -> Derived {
        ASSERT(rhs != 0.0);
        lhs /= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator-(Derived lhs) noexcept -> Derived {
        return Derived{-lhs.value_};
    }

  private:
    [[nodiscard]] explicit constexpr ValueObjectMixin(const double value) noexcept
        : value_{value} {}
    double value_;
};
}  // namespace abm::value_object
namespace abm {
class Wage;
class Money final : public value_object::ValueObjectMixin<Money> {
  public:
    [[nodiscard]] explicit constexpr Money(const double value) noexcept
        : ValueObjectMixin<Money>::ValueObjectMixin(value) {}

    [[nodiscard]] explicit constexpr operator Wage() const noexcept;
};

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