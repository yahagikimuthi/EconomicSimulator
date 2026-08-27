#pragma once

#include "core/assertion.hpp"
#include "core/values/mixin.hpp"

namespace abm {
class Wage;
class Deposit;
class Money final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Money>,
                    value_object::AddMixin<Money>,
                    value_object::ScholarMixin<Money> {
    friend class AddMixin<Money>;
    friend class ScholarMixin<Money>;

  public:
    [[nodiscard]] explicit constexpr Money(const double value) noexcept
        : BaseValueObject<double>(value) {}

    [[nodiscard]] explicit constexpr operator Wage() const noexcept;
    [[nodiscard]] explicit constexpr operator Deposit() const noexcept;
};

class AgentID final : public value_object::BaseValueObject<int>,
                      public value_object::CompareMixin<AgentID> {
  public:
    [[nodiscard]] explicit constexpr AgentID(const int value) noexcept
        : BaseValueObject<int>(value) {}
};

class Step final : public value_object::BaseValueObject<unsigned int>,
                   value_object::CompareMixin<Step> {
  public:
    [[nodiscard]] explicit constexpr Step(const unsigned int value) noexcept
        : BaseValueObject<unsigned int>(value) {}

    constexpr auto operator++() noexcept -> Step& {
        ++value_;
        return *this;
    }
    [[nodiscard]] constexpr auto operator%(const int other) const noexcept -> Step {
        ASSERT(other != 0);
        auto ret = static_cast<int>(value_) % other;
        return Step{static_cast<unsigned int>(ret)};
    }
};
}  // namespace abm