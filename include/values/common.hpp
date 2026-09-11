#pragma once

#include "values/mixin.hpp"

namespace abm::value_object {
class Wage;
class Budget;
class Money final : public BaseValueObject<double>,
                    CompareMixin<Money>,
                    AddMixin<Money>,
                    ScholarMixin<Money>,
                    public SignMixin {
    friend struct AddMixin<Money>;
    friend struct ScholarMixin<Money>;

  public:
    explicit constexpr Money(const double value) noexcept : BaseValueObject<double>(value) {}

    explicit constexpr operator Wage() const noexcept;
    explicit constexpr operator Budget() const noexcept;
};

class Budget final : public BaseValueObject<double>,
                     CompareMixin<Budget>,
                     AddMixin<Budget>,
                     ScholarMixin<Budget>,
                     public SignMixin {
    friend struct AddMixin<Budget>;
    friend struct ScholarMixin<Budget>;

  public:
    explicit constexpr Budget(const double value) noexcept : BaseValueObject<double>(value) {}

    operator Money() const noexcept = delete("予算とお金は別物！取引には使えない！");
};

constexpr Money::operator Budget() const noexcept { return Budget{value_}; }

class AgentID final : public BaseValueObject<int>, public CompareMixin<AgentID> {
  public:
    explicit constexpr AgentID(const int value) noexcept : BaseValueObject<int>(value) {}
};
}  // namespace abm::value_object

namespace abm {
using Money   = value_object::Money;
using Budget  = value_object::Budget;
using AgentID = value_object::AgentID;
}  // namespace abm