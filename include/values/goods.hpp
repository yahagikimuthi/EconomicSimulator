#pragma once

#include "core/assertion.hpp"
#include "values/common.hpp"
#include "values/mixin.hpp"

namespace abm {
class Price final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Price>,
                    value_object::AddMixin<Price>,
                    value_object::ScholarMixin<Price> {
    friend class AddMixin<Price>;
    friend class ScholarMixin<Price>;

  public:
    [[nodiscard]] constexpr explicit Price(const double value) noexcept
        : BaseValueObject<double>(value) {}
    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

class GoodsQuantity final : public value_object::BaseValueObject<double>,
                            value_object::CompareMixin<GoodsQuantity>,
                            value_object::AddMixin<GoodsQuantity>,
                            value_object::ScholarMixin<GoodsQuantity> {
    friend class AddMixin<GoodsQuantity>;
    friend class ScholarMixin<GoodsQuantity>;

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

class MarkupRate final : public value_object::BaseValueObject<double>,
                         value_object::CompareMixin<MarkupRate>,
                         value_object::AddMixin<MarkupRate>,
                         value_object::ScholarMixin<MarkupRate> {
    friend class AddMixin<MarkupRate>;
    friend class ScholarMixin<MarkupRate>;

  public:
    [[nodiscard]] explicit constexpr MarkupRate(const double value) noexcept
        : BaseValueObject<double>(value) {}
};

[[nodiscard]] constexpr auto operator*(Money lhs, MarkupRate rhs) noexcept -> Price {
    return Price{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator*(MarkupRate lhs, Money rhs) noexcept -> Price {
    return rhs * lhs;
}
}  // namespace abm