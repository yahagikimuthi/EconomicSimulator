#pragma once

#include "values/mixin.hpp"

namespace abm {
class Wage;
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
};

class AgentID final : public value_object::BaseValueObject<int>,
                      public value_object::CompareMixin<AgentID> {
  public:
    explicit constexpr AgentID(const int value) noexcept : BaseValueObject<int>(value) {}
};
}  // namespace abm