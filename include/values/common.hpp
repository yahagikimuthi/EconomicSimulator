#pragma once

#include "values/mixin.hpp"

namespace abm {
class Wage;
class Budget;
class Money final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Money>,
                    value_object::AddMixin<Money>,
                    value_object::ScholarMixin<Money>,
                    public value_object::SignMixin<Money> {
    friend struct AddMixin<Money>;
    friend struct ScholarMixin<Money>;

  public:
    explicit constexpr Money(const double value) noexcept : BaseValueObject<double>(value) {}

    explicit constexpr operator Wage() const noexcept;
    explicit constexpr operator Budget() const noexcept;
};

class Budget final : public value_object::BaseValueObject<double>,
                     value_object::CompareMixin<Budget>,
                     value_object::AddMixin<Budget>,
                     value_object::ScholarMixin<Budget>,
                     public value_object::SignMixin<Budget> {
    friend struct AddMixin<Budget>;
    friend struct ScholarMixin<Budget>;

  public:
    explicit constexpr Budget(const double value) noexcept : BaseValueObject<double>(value) {}
};

constexpr Money::operator Budget() const noexcept { return Budget{value_}; }

class AgentID final : public value_object::BaseValueObject<int>,
                      public value_object::CompareMixin<AgentID> {
  public:
    explicit constexpr AgentID(const int value) noexcept : BaseValueObject<int>(value) {}
};
}  // namespace abm