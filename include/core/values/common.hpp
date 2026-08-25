#pragma once

#include "core/values/mixin.hpp"

namespace abm {
class Wage;
class Money final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Money>,
                    value_object::AddMixin<Money>,
                    value_object::ScholarMixin<Money> {
    friend class value_object::CompareMixin<Money>;
    friend class value_object::AddMixin<Money>;
    friend class value_object::ScholarMixin<Money>;

  public:
    [[nodiscard]] explicit constexpr Money(const double value) noexcept
        : BaseValueObject<double>(value) {}

    [[nodiscard]] explicit constexpr operator Wage() const noexcept;
};

class AgentID final : public value_object::BaseValueObject<int>,
                      public value_object::CompareMixin<AgentID> {
    friend class value_object::CompareMixin<AgentID>;

  public:
    [[nodiscard]] explicit constexpr AgentID(const int value) noexcept
        : BaseValueObject<int>(value) {}
};

class Step final : public value_object::BaseValueObject<int>, value_object::CompareMixin<Step> {
    friend class value_object::CompareMixin<Step>;

  public:
    [[nodiscard]] explicit constexpr Step(const int value) noexcept : BaseValueObject<int>(value) {}

    constexpr auto operator++() -> Step& {
        ++value_;
        return *this;
    }
    [[nodiscard]] constexpr auto operator%(const int other) const noexcept -> Step {
        return Step{value_ % other};
    }
};
}  // namespace abm