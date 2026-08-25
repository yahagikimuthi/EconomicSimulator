#pragma once

#include <compare>

#include "core/assertion.hpp"

namespace abm::value_object {
template <typename T>
class BaseValueObject {
  public:
    [[nodiscard]] constexpr auto value() const noexcept -> T { return value_; }

  protected:
    [[nodiscard]] explicit constexpr BaseValueObject(const T value) noexcept : value_{value} {}

    T value_;
};

template <typename Derived>
class CompareMixin {
    friend Derived;

  public:
    [[nodiscard]] friend constexpr auto operator<=>(Derived lhs, Derived rhs) noexcept -> auto {
        return lhs.value_ <=> rhs.value_;
    }
    [[nodiscard]] friend constexpr auto operator==(Derived lhs, Derived rhs) noexcept -> bool {
        return lhs.value_ == rhs.value_;
    }

  private:
    [[nodiscard]] explicit constexpr CompareMixin() noexcept = default;
};

template <typename Derived>
class AddMixin {
    friend Derived;

  public:
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
    [[nodiscard]] explicit constexpr AddMixin() noexcept = default;
};

template <typename Derived>
class ScholarMixin {
    friend Derived;

  public:
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
    [[nodiscard]] explicit constexpr ScholarMixin() noexcept = default;
};
}  // namespace abm::value_object