#pragma once

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/mixin.hpp"

namespace abm {
class Price final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Price>,
                    value_object::AddMixin<Price>,
                    value_object::ScholarMixin<Price>,
                    public value_object::SignMixin {
    friend struct AddMixin<Price>;
    friend struct ScholarMixin<Price>;

  public:
    [[nodiscard]] constexpr explicit Price(const double value) noexcept
        : BaseValueObject<double>(value) {}
    explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

class GoodsQuantity final : public value_object::BaseValueObject<double>,
                            value_object::CompareMixin<GoodsQuantity>,
                            value_object::AddMixin<GoodsQuantity>,
                            value_object::ScholarMixin<GoodsQuantity>,
                            public value_object::SignMixin {
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
                         public value_object::SignMixin {
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
}  // namespace abm