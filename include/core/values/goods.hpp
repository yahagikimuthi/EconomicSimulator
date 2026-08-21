#pragma once

#include <compare>

#include "core/assertion.hpp"
#include "core/values/common.hpp"

namespace abm {
class Price final {
  public:
    [[nodiscard]] constexpr explicit Price(const double value) noexcept : value_{value} {}
    [[nodiscard]] constexpr auto value() const noexcept -> double { return value_; }
    [[nodiscard]] constexpr auto operator<=>(const Price&) const noexcept -> auto = default;

    constexpr auto operator+=(const Price other) noexcept -> Price& {
        value_ += other.value();
        return *this;
    }
    constexpr auto operator-=(const Price other) noexcept -> Price& {
        value_ -= other.value();
        return *this;
    }
    constexpr auto operator*=(const double other) noexcept -> Price& {
        value_ += other;
        return *this;
    }
    constexpr auto operator/=(const double other) noexcept -> Price& {
        ASSERT(other != 0.0);
        value_ /= other;
        return *this;
    }
    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }

  private:
    double value_;
};

[[nodiscard]] constexpr auto operator+(Price lhs, const Price rhs) noexcept -> Price {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator-(Price lhs, const Price rhs) noexcept -> Price {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(Price lhs, const double rhs) noexcept -> Price {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(const double lhs, const Price rhs) noexcept -> Price {
    return rhs * lhs;
}
[[nodiscard]] constexpr auto operator/(Price lhs, const double rhs) noexcept -> Price {
    ASSERT(rhs != 0.0);
    lhs /= rhs;
    return lhs;
}

class GoodsQuantity final {
  public:
    [[nodiscard]] explicit constexpr GoodsQuantity(const double value) noexcept : value_{value} {}
    [[nodiscard]] auto value() const noexcept -> double { return value_; }
    [[nodiscard]] auto operator<=>(const GoodsQuantity&) const noexcept -> auto = default;

    constexpr auto operator+=(const GoodsQuantity other) noexcept -> GoodsQuantity& {
        value_ += other.value();
        return *this;
    }
    constexpr auto operator-=(const GoodsQuantity other) noexcept -> GoodsQuantity& {
        value_ -= other.value();
        return *this;
    }
    constexpr auto operator/(const GoodsQuantity other) const noexcept -> double {
        ASSERT(other != GoodsQuantity{0.0});
        return value_ / other.value();
    }
    constexpr auto operator*=(const double other) noexcept -> GoodsQuantity& {
        value_ += other;
        return *this;
    }
    constexpr auto operator/=(const double other) noexcept -> GoodsQuantity& {
        ASSERT(other != 0.0);
        value_ /= other;
        return *this;
    }

  private:
    double value_;
};
[[nodiscard]] constexpr auto operator+(GoodsQuantity lhs, GoodsQuantity rhs) noexcept
    -> GoodsQuantity {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator-(GoodsQuantity lhs, GoodsQuantity rhs) noexcept
    -> GoodsQuantity {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(GoodsQuantity lhs, double rhs) noexcept -> GoodsQuantity {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(double lhs, GoodsQuantity rhs) noexcept -> GoodsQuantity {
    return rhs * lhs;
}
[[nodiscard]] constexpr auto operator/(GoodsQuantity lhs, double rhs) noexcept -> GoodsQuantity {
    ASSERT(rhs != 0.0);
    lhs /= rhs;
    return lhs;
}

[[nodiscard]] constexpr auto operator*(Price lhs, GoodsQuantity rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, Price rhs) noexcept -> GoodsQuantity {
    ASSERT(rhs != Price{0.0});
    return GoodsQuantity{lhs.value() / rhs.value()};
}
[[nodiscard]] constexpr auto operator/(Money lhs, GoodsQuantity rhs) noexcept -> Price {
    ASSERT(rhs != GoodsQuantity{0.0});
    return Price{lhs.value() / rhs.value()};
}
}  // namespace abm