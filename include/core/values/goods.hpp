#pragma once

#include "core/assertion.hpp"
#include "core/values/common.hpp"

namespace abm {
class Price final : public ValueObjectMixin<Price> {
  public:
    [[nodiscard]] constexpr explicit Price(const double value) noexcept
        : ValueObjectMixin<Price>::ValueObjectMixin(value) {}
    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

class GoodsQuantity final : public ValueObjectMixin<GoodsQuantity> {
  public:
    [[nodiscard]] explicit constexpr GoodsQuantity(const double value) noexcept
        : ValueObjectMixin<GoodsQuantity>::ValueObjectMixin(value) {}
};

[[nodiscard]] constexpr auto operator*(Price lhs, GoodsQuantity rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, Price rhs) noexcept -> GoodsQuantity {
    ASSERT(rhs != Price{0.0});
    return GoodsQuantity{lhs.value() / rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, GoodsQuantity rhs) noexcept -> Price {
    ASSERT(rhs != GoodsQuantity{0.0});
    return Price{lhs.value() / rhs.value()};
}
}  // namespace abm