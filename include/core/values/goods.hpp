#pragma once

#include "core/assertion.hpp"
#include "core/values/common.hpp"
#include "core/values/mixin.hpp"

namespace abm {
class Price final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Price>,
                    value_object::AddMixin<Price>,
                    value_object::ScholarMixin<Price> {
    friend class value_object::CompareMixin<Price>;
    friend class value_object::AddMixin<Price>;
    friend class value_object::ScholarMixin<Price>;

  public:
    [[nodiscard]] constexpr explicit Price(const double value) noexcept
        : BaseValueObject<double>(value) {}
    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

class GoodsQuantity final : public value_object::BaseValueObject<double>,
                            value_object::CompareMixin<GoodsQuantity>,
                            value_object::AddMixin<GoodsQuantity>,
                            value_object::ScholarMixin<GoodsQuantity> {
    friend class value_object::CompareMixin<GoodsQuantity>;
    friend class value_object::AddMixin<GoodsQuantity>;
    friend class value_object::ScholarMixin<GoodsQuantity>;

  public:
    [[nodiscard]] explicit constexpr GoodsQuantity(const double value) noexcept
        : BaseValueObject<double>(value) {}
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