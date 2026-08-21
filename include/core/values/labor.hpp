#pragma once

#include <cmath>
#include <compare>

#include "core/values/common.hpp"

namespace abm {
class [[nodiscard]] Wage final : public ValueObjectMixin<Wage> {
  public:
    [[nodiscard]] explicit constexpr Wage(const double value) noexcept
        : ValueObjectMixin<Wage>::ValueObjectMixin(value) {}
    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

class [[nodiscard]] HeadCount final : public ValueObjectMixin<HeadCount> {
  public:
    [[nodiscard]] explicit constexpr HeadCount(const double value) noexcept
        : ValueObjectMixin<HeadCount>::ValueObjectMixin(value) {}
    [[nodiscard]] explicit constexpr HeadCount(const int value) noexcept
        : ValueObjectMixin<HeadCount>::ValueObjectMixin(static_cast<double>(value)) {}
    [[nodiscard]] explicit constexpr HeadCount(const std::size_t value) noexcept
        : ValueObjectMixin<HeadCount>::ValueObjectMixin(static_cast<double>(value)) {}

    constexpr auto ceil() noexcept -> HeadCount& {
        value_ = std::ceil(value_);
        return *this;
    }
    constexpr auto operator++() noexcept -> HeadCount& {
        ++value_;
        return *this;
    }
    constexpr auto operator--() noexcept -> HeadCount& {
        --value_;
        return *this;
    }
};
[[nodiscard]] constexpr auto operator*(Wage lhs, HeadCount rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, Wage rhs) noexcept -> HeadCount {
    ASSERT(rhs != Wage{0.0});
    return HeadCount{lhs.value() / rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, HeadCount rhs) noexcept -> Wage {
    ASSERT(rhs != HeadCount{0.0});
    return Wage{lhs.value() / rhs.value()};
}
}  // namespace abm