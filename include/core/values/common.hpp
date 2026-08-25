#pragma once

#include <compare>

#include "core/assertion.hpp"

namespace abm::value_object {
template <typename T>
class BaseValueObjectMixin {
  public:
    [[nodiscard]] constexpr auto value() const noexcept -> T { return value_; }

  protected:
    [[nodiscard]] explicit constexpr BaseValueObjectMixin(const T value) : value_{value} {}

    T value_;
};

template <typename Derived>
class ComparableMixin {
    friend Derived;

  public:
    [[nodiscard]] friend constexpr auto operator<=>(Derived lhs, Derived rhs) noexcept -> auto {
        return lhs.value_ <=> rhs.value_;
    }
    [[nodiscard]] friend constexpr auto operator==(Derived lhs, Derived rhs) noexcept -> bool {
        return lhs.value_ == rhs.value_;
    }

  private:
    [[nodiscard]] explicit constexpr ComparableMixin() = default;
};

template <typename Derived>
class ScholarMixin {
    friend Derived;

  public:
    friend constexpr auto operator+=(Derived& lhs, Derived rhs) noexcept -> Derived& {
        lhs.value_ += rhs.value_;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator+(Derived lhs, Derived rhs) noexcept -> Derived {
        lhs += rhs;
        return lhs;
    }
    friend constexpr auto operator-=(Derived& lhs, Derived rhs) noexcept -> Derived& {
        lhs.value_ -= rhs.value_;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator-(Derived lhs, Derived rhs) noexcept -> Derived {
        lhs -= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator/(Derived lhs, Derived rhs) noexcept -> double {
        ASSERT(rhs.value_ != 0.0);
        return lhs.value_ / rhs.value_;
    }
    friend constexpr auto operator*=(Derived& lhs, double rhs) noexcept -> Derived& {
        lhs.value_ *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(Derived lhs, double rhs) noexcept -> Derived {
        lhs *= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator*(double lhs, Derived rhs) noexcept -> Derived {
        return rhs * lhs;
    }
    friend constexpr auto operator/=(Derived& lhs, double rhs) noexcept -> Derived& {
        ASSERT(rhs != 0.0);
        lhs.value_ /= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator/(Derived lhs, double rhs) noexcept -> Derived {
        ASSERT(rhs != 0.0);
        lhs /= rhs;
        return lhs;
    }
    [[nodiscard]] friend constexpr auto operator-(Derived lhs) noexcept -> Derived {
        return Derived{-lhs.value_};
    }

  private:
    [[nodiscard]] explicit constexpr ScholarMixin() = default;
};
}  // namespace abm::value_object

namespace abm {
class Wage;
class Money final : public value_object::BaseValueObjectMixin<double>,
                    public value_object::ComparableMixin<Money>,
                    public value_object::ScholarMixin<Money> {
    friend value_object::ComparableMixin<Money>;
    friend value_object::ScholarMixin<Money>;

  public:
    [[nodiscard]] explicit constexpr Money(const double value) noexcept
        : BaseValueObjectMixin<double>(value) {}

    [[nodiscard]] explicit constexpr operator Wage() const noexcept;
};

class AgentID final : public value_object::BaseValueObjectMixin<int>,
                      public value_object::ComparableMixin<AgentID> {
    friend class value_object::ComparableMixin<AgentID>;

  public:
    [[nodiscard]] explicit constexpr AgentID(const int value) noexcept
        : BaseValueObjectMixin<int>(value) {}
};

class Step final : public value_object::BaseValueObjectMixin<int>,
                   public value_object::ComparableMixin<Step> {
    friend class value_object::ComparableMixin<Step>;

  public:
    [[nodiscard]] explicit constexpr Step(const int value) noexcept
        : BaseValueObjectMixin<int>(value) {}

    constexpr auto operator++() -> Step& {
        ++value_;
        return *this;
    }
    [[nodiscard]] constexpr auto operator%(const int other) const noexcept -> Step {
        return Step{value_ % other};
    }
};
}  // namespace abm