#pragma once

#include <cassert>

#include "others/type.hpp"
#include "values/common.hpp"
#include "values/mixin.hpp"

namespace abm::value_object {
class Wage final : public BaseValueObject<f64>,
                   CompareMixin<Wage>,
                   AddMixin<Wage>,
                   ScholarMixin<Wage>,
                   public SignMixin {
    friend struct AddMixin<Wage>;
    friend struct ScholarMixin<Wage>;

  public:
    explicit constexpr Wage(const f64 value) noexcept : BaseValueObject<f64>(value) {}
    explicit constexpr operator Money() const noexcept { return Money{value_}; }
    explicit constexpr operator Budget() const noexcept { return Budget{value_}; }
};

constexpr Money::operator Wage() const noexcept { return Wage{value_}; }

class HeadCount final : public BaseValueObject<f64>,
                        CompareMixin<HeadCount>,
                        AddMixin<HeadCount>,
                        ScholarMixin<HeadCount>,
                        public SignMixin {
    friend struct AddMixin<HeadCount>;
    friend struct ScholarMixin<HeadCount>;

  public:
    explicit constexpr HeadCount(const f64 value) noexcept : BaseValueObject<f64>(value) {}
    explicit constexpr HeadCount(const i32 value) noexcept
        : BaseValueObject<f64>(static_cast<f64>(value)) {}
    explicit constexpr HeadCount(const std::size_t value) noexcept
        : BaseValueObject<f64>(static_cast<f64>(value)) {}

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
[[nodiscard]] constexpr auto operator*(HeadCount lhs, Wage rhs) noexcept -> Money {
    return rhs * lhs;
}
[[nodiscard]] constexpr auto operator/(Money lhs, Wage rhs) noexcept -> HeadCount {
    assert(rhs != Wage{0.0});
    return HeadCount{lhs.value() / rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, HeadCount rhs) noexcept -> Wage {
    assert(rhs != HeadCount{0.0});
    return Wage{lhs.value() / rhs.value()};
}

class OfferRate final : public BaseValueObject<f64>,
                        public CompareMixin<OfferRate>,
                        AddMixin<OfferRate>,
                        ScholarMixin<OfferRate>,
                        public SignMixin {
    friend struct AddMixin<OfferRate>;
    friend struct ScholarMixin<OfferRate>;

  public:
    explicit constexpr OfferRate(const f64 value) noexcept : BaseValueObject<f64>(value) {}
};

[[nodiscard]] constexpr auto operator*(HeadCount lhs, OfferRate rhs) noexcept -> HeadCount {
    return HeadCount{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator*(OfferRate lhs, HeadCount rhs) noexcept -> HeadCount {
    return rhs * lhs;
}
}  // namespace abm::value_object

namespace abm {
using HeadCount = value_object::HeadCount;
using Wage      = value_object::Wage;
using OfferRate = value_object::OfferRate;
}  // namespace abm