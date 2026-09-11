#pragma once

#include <cassert>

#include "values/common.hpp"
#include "values/mixin.hpp"

namespace abm::value_object {
class Price final : public BaseValueObject<double>,
                    CompareMixin<Price>,
                    AddMixin<Price>,
                    ScholarMixin<Price>,
                    public SignMixin {
    friend struct AddMixin<Price>;
    friend struct ScholarMixin<Price>;

  public:
    [[nodiscard]] constexpr explicit Price(const double value) noexcept
        : BaseValueObject<double>(value) {}
    explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

class GoodsQuantity final : public BaseValueObject<double>,
                            CompareMixin<GoodsQuantity>,
                            AddMixin<GoodsQuantity>,
                            ScholarMixin<GoodsQuantity>,
                            public SignMixin {
    friend struct AddMixin<GoodsQuantity>;
    friend struct ScholarMixin<GoodsQuantity>;

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
    assert(rhs != Price{0.0});
    return GoodsQuantity{lhs.value() / rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, GoodsQuantity rhs) noexcept -> Price {
    assert(rhs != GoodsQuantity{0.0});
    return Price{lhs.value() / rhs.value()};
}

class MarkupRate final : public BaseValueObject<double>,
                         CompareMixin<MarkupRate>,
                         AddMixin<MarkupRate>,
                         ScholarMixin<MarkupRate>,
                         public SignMixin {
    friend struct AddMixin<MarkupRate>;
    friend struct ScholarMixin<MarkupRate>;

  public:
    explicit constexpr MarkupRate(const double value) noexcept : BaseValueObject<double>(value) {}
};

[[nodiscard]] constexpr auto operator*(Money lhs, MarkupRate rhs) noexcept -> Price {
    return Price{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator*(MarkupRate lhs, Money rhs) noexcept -> Price {
    return rhs * lhs;
}
}  // namespace abm::value_object

namespace abm {
using Price         = value_object::Price;
using GoodsQuantity = value_object::GoodsQuantity;
using MarkupRate    = value_object::MarkupRate;
}  // namespace abm