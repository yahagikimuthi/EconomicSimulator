#pragma once

#include "core/assertion.hpp"
#include "core/values/common.hpp"
#include "core/values/mixin.hpp"

namespace abm {
class TaxRate final : public value_object::BaseValueObject<double>,
                      value_object::CompareMixin<TaxRate>,
                      value_object::AddMixin<TaxRate>,
                      value_object::ScholarMixin<TaxRate> {
    friend class CompareMixin<TaxRate>;
    friend class AddMixin<TaxRate>;
    friend class ScholarMixin<TaxRate>;

  public:
    [[nodiscard]] explicit constexpr TaxRate(const double value) noexcept
        : BaseValueObject<double>(value) {
        ASSERT(0.0 <= value and value <= 1.0);
    }
};

[[nodiscard]] constexpr auto operator*(Money lhs, TaxRate rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}

[[nodiscard]] constexpr auto operator*(TaxRate lhs, Money rhs) noexcept -> Money {
    return rhs * lhs;
}
}  // namespace abm