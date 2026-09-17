#pragma once

#include <cassert>

#include "others/type.hpp"
#include "values/common.hpp"
#include "values/mixin.hpp"

namespace abm::value_object {
class TaxRate final : public BaseValueObject<f64>,
                      CompareMixin<TaxRate>,
                      AddMixin<TaxRate>,
                      ScholarMixin<TaxRate>,
                      public SignMixin {
    friend struct AddMixin<TaxRate>;
    friend struct ScholarMixin<TaxRate>;

  public:
    explicit constexpr TaxRate(const f64 value) noexcept : BaseValueObject<f64>(value) {
        assert(0.0 <= value and value <= 1.0);
    }
};

[[nodiscard]] constexpr auto operator*(Money lhs, TaxRate rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}

[[nodiscard]] constexpr auto operator*(TaxRate lhs, Money rhs) noexcept -> Money {
    return rhs * lhs;
}
}  // namespace abm::value_object

namespace abm {
using TaxRate = value_object::TaxRate;
}