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

class Deposit final : public value_object::BaseValueObject<double>,
                      value_object::CompareMixin<Deposit>,
                      value_object::AddMixin<Deposit>,
                      value_object::ScholarMixin<Deposit> {
    friend class CompareMixin<Deposit>;
    friend class AddMixin<Deposit>;
    friend class ScholarMixin<Deposit>;

  public:
    [[nodiscard]] explicit constexpr Deposit(const double value) noexcept
        : BaseValueObject<double>(value) {}
};

[[nodiscard]] constexpr Money::operator Deposit() const noexcept { return Deposit{value_}; }

class InterestRate final : public value_object::BaseValueObject<double>,
                           value_object::CompareMixin<InterestRate>,
                           value_object::AddMixin<InterestRate>,
                           value_object::ScholarMixin<InterestRate> {
    friend class CompareMixin<InterestRate>;
    friend class AddMixin<InterestRate>;
    friend class ScholarMixin<InterestRate>;

  public:
    [[nodiscard]] explicit constexpr InterestRate(const double value) noexcept
        : BaseValueObject<double>(value) {}
};

[[nodiscard]] constexpr auto operator*(Deposit lhs, InterestRate rhs) noexcept -> Deposit {
    return Deposit{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator*(InterestRate lhs, Deposit rhs) noexcept -> Deposit {
    return rhs * lhs;
}

}  // namespace abm