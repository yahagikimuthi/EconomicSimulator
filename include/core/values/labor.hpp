#pragma once

#include "core/assertion.hpp"
#include "core/values/common.hpp"
#include "core/values/mixin.hpp"

namespace abm {
class [[nodiscard]] Wage final : public value_object::BaseValueObject<double>,
                                 value_object::CompareMixin<Wage>,
                                 value_object::AddMixin<Wage>,
                                 value_object::ScholarMixin<Wage> {
    friend class CompareMixin<Wage>;
    friend class AddMixin<Wage>;
    friend class ScholarMixin<Wage>;

  public:
    [[nodiscard]] explicit constexpr Wage(const double value) noexcept
        : BaseValueObject<double>(value) {}
    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }
};

constexpr Money::operator Wage() const noexcept { return Wage{value_}; }

class [[nodiscard]] HeadCount final : public value_object::BaseValueObject<double>,
                                      value_object::CompareMixin<HeadCount>,
                                      value_object::AddMixin<HeadCount>,
                                      value_object::ScholarMixin<HeadCount> {
    friend class CompareMixin<HeadCount>;
    friend class AddMixin<HeadCount>;
    friend class ScholarMixin<HeadCount>;

  public:
    [[nodiscard]] explicit constexpr HeadCount(const double value) noexcept
        : BaseValueObject<double>(value) {}
    [[nodiscard]] explicit constexpr HeadCount(const int value) noexcept
        : BaseValueObject<double>(static_cast<double>(value)) {}
    [[nodiscard]] explicit constexpr HeadCount(const std::size_t value) noexcept
        : BaseValueObject<double>(static_cast<double>(value)) {}

    constexpr auto operator++() noexcept -> HeadCount& {
        ++value_;
        return *this;
    }
    constexpr auto operator--() noexcept -> HeadCount& {
        --value_;
        return *this;
    }
};
[[nodiscard]] constexpr auto operator*(Wage lhs, HeadCount rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, Wage rhs) noexcept -> HeadCount {
    ASSERT(rhs != Wage{0.0});
    return HeadCount{lhs.value() / rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, HeadCount rhs) noexcept -> Wage {
    ASSERT(rhs != HeadCount{0.0});
    return Wage{lhs.value() / rhs.value()};
}
}  // namespace abm