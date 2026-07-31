#pragma once

#include <compare>

#include "core/base.hpp"
#include "core/values/common.hpp"

class [[nodiscard]] Price : public internal::BaseValueClass<double> {
  public:
    using internal::BaseValueClass<double>::BaseValueClass;
    Price() : internal::BaseValueClass<double>::BaseValueClass(0.0) {}
    auto operator<=>(const Price&) const = default;
    auto operator+=(const Price other) -> Price& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const Price other) -> Price& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) -> Price& {
        value_ += other;
        return *this;
    }
    auto operator/=(const double other) -> Price& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }
};
[[nodiscard]] inline auto operator+(Price lhs, const Price rhs) -> Price {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(Price lhs, const Price rhs) -> Price {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(Price lhs, const double rhs) -> Price {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(const double lhs, const Price rhs) -> Price {
    return rhs * lhs;
}
[[nodiscard]] inline auto operator/(Price lhs, const double rhs) -> Price {
    lhs /= rhs;
    return lhs;
}

class [[nodiscard]] GoodsQuantity : public internal::BaseValueClass<double> {
  public:
    using internal::BaseValueClass<double>::BaseValueClass;
    GoodsQuantity() : internal::BaseValueClass<double>::BaseValueClass(0.0) {}
    auto operator<=>(const GoodsQuantity&) const = default;
    auto operator+=(const GoodsQuantity other) -> GoodsQuantity& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const GoodsQuantity other) -> GoodsQuantity& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) -> GoodsQuantity& {
        value_ += other;
        return *this;
    }
    auto operator/=(const double other) -> GoodsQuantity& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }
};
[[nodiscard]] inline auto operator+(GoodsQuantity lhs, GoodsQuantity rhs) -> GoodsQuantity {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(GoodsQuantity lhs, GoodsQuantity rhs) -> GoodsQuantity {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(GoodsQuantity lhs, double rhs) -> GoodsQuantity {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(double lhs, GoodsQuantity rhs) -> GoodsQuantity {
    return rhs * lhs;
}
[[nodiscard]] inline auto operator/(GoodsQuantity lhs, double rhs) -> GoodsQuantity
    PRE(rhs != 0.0) {
    lhs /= rhs;
    return lhs;
}

[[nodiscard]] inline auto operator*(Price lhs, GoodsQuantity rhs) -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] inline auto operator/(Money lhs, Price rhs) -> Money PRE(rhs != 0.0) {
    return Money{lhs.value() / rhs.value()};
}
[[nodiscard]] inline auto operator/(Money lhs, GoodsQuantity rhs) -> Money PRE(rhs != 0.0) {
    return Money{lhs.value() / rhs.value()};
}