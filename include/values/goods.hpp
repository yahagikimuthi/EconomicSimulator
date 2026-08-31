#pragma once

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/mixin.hpp"

namespace abm {
class Price final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Price>,
                    value_object::AddMixin<Price>,
                    value_object::ScholarMixin<Price>,
                    value_object::SignMixin<Price> {
    friend class AddMixin<Price>;
    friend class ScholarMixin<Price>;

  public:
    [[nodiscard]] constexpr explicit Price(const double value) noexcept
        : BaseValueObject<double>(value) {}
    explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

class GoodsQuantity final : public value_object::BaseValueObject<double>,
                            value_object::CompareMixin<GoodsQuantity>,
                            value_object::AddMixin<GoodsQuantity>,
                            value_object::ScholarMixin<GoodsQuantity>,
                            public value_object::SignMixin<GoodsQuantity> {
    friend class AddMixin<GoodsQuantity>;
    friend class ScholarMixin<GoodsQuantity>;

  public:
    explicit constexpr GoodsQuantity(const double value) noexcept
        : BaseValueObject<double>(value) {}
};

[[nodiscard]] constexpr auto operator*(Price lhs, GoodsQuantity rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator*(GoodsQuantity lhs, Price rhs) noexcept -> Money {
    return rhs * lhs;
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
                         value_object::ScholarMixin<MarkupRate>,
                         value_object::SignMixin<MarkupRate> {
    friend class AddMixin<MarkupRate>;
    friend class ScholarMixin<MarkupRate>;

  public:
    explicit constexpr MarkupRate(const double value) noexcept : BaseValueObject<double>(value) {}
};

[[nodiscard]] constexpr auto operator*(Money lhs, MarkupRate rhs) noexcept -> Price {
    return Price{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator*(MarkupRate lhs, Money rhs) noexcept -> Price {
    return rhs * lhs;
}
}  // namespace abm