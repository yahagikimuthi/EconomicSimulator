#pragma once

#include "core/assertion.hpp"
#include "core/values/common.hpp"

namespace abm {
class [[nodiscard]] Wage final : public value_object::ValueObjectMixin<Wage> {
  public:
    [[nodiscard]] explicit constexpr Wage(const double value) noexcept
        : ValueObjectMixin<Wage>(value) {}
    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

constexpr Money::operator Wage() const noexcept { return Wage{value_}; }

class [[nodiscard]] HeadCount final : public value_object::ValueObjectMixin<HeadCount> {
  public:
    [[nodiscard]] explicit constexpr HeadCount(const double value) noexcept
        : ValueObjectMixin<HeadCount>(value) {}
    [[nodiscard]] explicit constexpr HeadCount(const int value) noexcept
        : ValueObjectMixin<HeadCount>(static_cast<double>(value)) {}
    [[nodiscard]] explicit constexpr HeadCount(const std::size_t value) noexcept
        : ValueObjectMixin<HeadCount>(static_cast<double>(value)) {}

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