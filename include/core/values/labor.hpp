#pragma once

#include <compare>

#include "core/values/common.hpp"

namespace abm {
class [[nodiscard]] Wage {
  public:
    [[nodiscard]] explicit Wage(const double value) : value_{value} {}
    [[nodiscard]] auto value() const -> double { return value_; }
    [[nodiscard]] auto operator<=>(const Wage&) const = default;
    auto               operator+=(const Wage other) -> Wage& {
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

    [[nodiscard]] explicit operator Money() const { return Money{value_}; }

  private:
    double value_;
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

class [[nodiscard]] HeadCount {
  public:
    [[nodiscard]] explicit HeadCount(const double value) : value_{value} {}
    [[nodiscard]] explicit HeadCount(const int value) : value_{static_cast<double>(value)} {}
    [[nodiscard]] explicit HeadCount(const std::size_t value)
        : value_{static_cast<double>(value)} {}
    [[nodiscard]] auto value() const -> double { return value_; }
    [[nodiscard]] auto operator<=>(const HeadCount&) const = default;
    auto               operator+=(const HeadCount other) -> HeadCount& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const HeadCount other) -> HeadCount& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) -> HeadCount& {
        value_ *= other;
        return *this;
    }
    auto operator/=(const double other) -> HeadCount& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }
    auto operator++() -> HeadCount& {
        ++value_;
        return *this;
    }
    auto operator--() -> HeadCount& {
        --value_;
        return *this;
    }

  private:
    double value_;
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
[[nodiscard]] inline auto operator/(Money lhs, Wage rhs) -> HeadCount PRE(rhs != Wage{0.0}) {
    return HeadCount{lhs.value() / rhs.value()};
}
[[nodiscard]] inline auto operator/(Money lhs, HeadCount rhs) -> Wage PRE(rhs != HeadCount{0.0}) {
    return Wage{lhs.value() / rhs.value()};
}
}  // namespace abm