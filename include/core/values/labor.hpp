#pragma once

#include <compare>

#include "core/values/common.hpp"

class [[nodiscard]] Wage : public internal::BaseValueClass<double> {
  public:
    using internal::BaseValueClass<double>::BaseValueClass;
    auto operator<=>(const Wage&) const = default;
    auto operator+=(const Wage other) -> Wage& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const Wage other) -> Wage& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) -> Wage& {
        value_ += other;
        return *this;
    }
    auto operator/=(const double other) -> Wage& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }
};
[[nodiscard]] inline auto operator+(Wage lhs, const Wage rhs) -> Wage {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(Wage lhs, const Wage rhs) -> Wage {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(Wage lhs, const double rhs) -> Wage {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(const double lhs, const Wage rhs) -> Wage { return rhs * lhs; }
[[nodiscard]] inline auto operator/(Wage lhs, const double rhs) -> Wage {
    lhs /= rhs;
    return lhs;
}

class [[nodiscard]] HeadCount : public internal::BaseValueClass<double> {
  public:
    using internal::BaseValueClass<double>::BaseValueClass;
    auto operator<=>(const HeadCount&) const = default;
    auto operator+=(const HeadCount other) -> HeadCount& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const HeadCount other) -> HeadCount& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) -> HeadCount& {
        value_ += other;
        return *this;
    }
    auto operator/=(const double other) -> HeadCount& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }
};
[[nodiscard]] inline auto operator+(HeadCount lhs, const HeadCount rhs) -> HeadCount {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(HeadCount lhs, const HeadCount rhs) -> HeadCount {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(HeadCount lhs, const double rhs) -> HeadCount {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(const double lhs, const HeadCount rhs) -> HeadCount {
    return rhs * lhs;
}
[[nodiscard]] inline auto operator/(HeadCount lhs, const double rhs) -> HeadCount {
    lhs /= rhs;
    return lhs;
}

[[nodiscard]] inline auto operator*(Wage lhs, HeadCount rhs) -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] inline auto operator/(Money lhs, Wage rhs) -> Money PRE(rhs != 0.0) {
    return Money{lhs.value() / rhs.value()};
}
[[nodiscard]] inline auto operator/(Money lhs, HeadCount rhs) -> Money PRE(rhs != 0.0) {
    return Money{lhs.value() / rhs.value()};
}