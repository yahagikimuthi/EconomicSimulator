#pragma once

#include <compare>

#include "others/util.hpp"

namespace abm::value_object {
template <typename T>
class BaseValueObject {
  public:
    [[nodiscard]] constexpr auto value() const noexcept -> T { return value_; }

  protected:
    explicit constexpr BaseValueObject(const T value) noexcept : value_{value} {}

    T value_;
};

template <typename Derived>
class CompareMixin {
    friend Derived;

    [[nodiscard]] friend constexpr auto operator<=>(Derived lhs, Derived rhs) noexcept -> auto {
        return lhs.value() <=> rhs.value();
    }
    [[nodiscard]] friend constexpr auto operator==(Derived lhs, Derived rhs) noexcept -> bool {
        return lhs.value() == rhs.value();
    }

  private:
    explicit constexpr CompareMixin() noexcept = default;
};

template <typename Derived>
class AddMixin {
    friend Derived;

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

  private:
    explicit constexpr AddMixin() noexcept = default;
};

template <typename Derived>
class ScholarMixin {
    friend Derived;

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
    explicit constexpr ScholarMixin() noexcept = default;
};

template <typename Derived>
struct SignMixin {
    friend Derived;

  public:
    [[nodiscard]] constexpr auto isPositive() const noexcept -> bool { return This() > Derived{0}; }
    [[nodiscard]] constexpr auto isZeroOrMore() const noexcept -> bool {
        return This() >= Derived{0};
    }
    [[nodiscard]] constexpr auto isZero() const noexcept -> bool { return This() == Derived{0}; }
    [[nodiscard]] constexpr auto isZeroOrLess() const noexcept -> bool {
        return This() <= Derived{0};
    }
    [[nodiscard]] constexpr auto isNegative() const noexcept -> bool { return This() < Derived{0}; }

  private:
    [[nodiscard]] constexpr auto This() const noexcept -> const Derived& {
        return static_cast<const Derived&>(*this);
    }
    [[nodiscard]] SignMixin() noexcept = default;
};
}  // namespace abm::value_object