#pragma once

#include "others/util.hpp"
#include "values/mixin.hpp"

namespace abm {
class Wage;
class Deposit;
class Money final : public value_object::BaseValueObject<double>,
                    value_object::CompareMixin<Money>,
                    value_object::AddMixin<Money>,
                    value_object::ScholarMixin<Money>,
                    public value_object::SignMixin<Money> {
    friend class AddMixin<Money>;
    friend class ScholarMixin<Money>;

  public:
    explicit constexpr Money(const double value) noexcept : BaseValueObject<double>(value) {}

    explicit constexpr operator Wage() const noexcept;
    explicit constexpr operator Deposit() const noexcept;
};

class AgentID final : public value_object::BaseValueObject<int>,
                      public value_object::CompareMixin<AgentID> {
  public:
    explicit constexpr AgentID(const int value) noexcept : BaseValueObject<int>(value) {}
};

class Step final : public value_object::BaseValueObject<unsigned int>,
                   value_object::CompareMixin<Step> {
  public:
    explicit constexpr Step(const unsigned int value) noexcept
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