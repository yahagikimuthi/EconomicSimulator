#pragma once

#include "others/type.hpp"
#include "values/mixin.hpp"

namespace abm::value_object {
class Wage;
class Budget;
class Money final : public BaseValueObject<f64>,
                    CompareMixin<Money>,
                    AddMixin<Money>,
                    ScholarMixin<Money>,
                    public SignMixin {
    friend struct AddMixin<Money>;
    friend struct ScholarMixin<Money>;

  public:
    explicit constexpr Money(const f64 value) noexcept : BaseValueObject<f64>(value) {}

    explicit constexpr operator Wage() const noexcept;
    explicit constexpr operator Budget() const noexcept;
};

class Budget final : public BaseValueObject<f64>,
                     CompareMixin<Budget>,
                     AddMixin<Budget>,
                     ScholarMixin<Budget>,
                     public SignMixin {
    friend struct AddMixin<Budget>;
    friend struct ScholarMixin<Budget>;

  public:
    explicit constexpr Budget(const f64 value) noexcept : BaseValueObject<f64>(value) {}

    operator Money() const noexcept = delete("予算とお金は別物！取引には使えない！");
};

constexpr Money::operator Budget() const noexcept { return Budget{value_}; }

class AgentID final : public BaseValueObject<i32>, public CompareMixin<AgentID> {
  public:
    explicit constexpr AgentID(const i32 value) noexcept : BaseValueObject<i32>(value) {}
};
}  // namespace abm::value_object

namespace abm {
using Money   = value_object::Money;
using Budget  = value_object::Budget;
using AgentID = value_object::AgentID;
}  // namespace abm