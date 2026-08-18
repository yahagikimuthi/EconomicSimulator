#pragma once

#include <compare>

#include "core/values/common.hpp"

namespace abm {
class [[nodiscard]] Wage final {
  public:
    [[nodiscard]] explicit Wage(const double value) noexcept : value_{value} {}
    [[nodiscard]] auto value() const noexcept -> double { return value_; }
    [[nodiscard]] auto operator<=>(const Wage&) const noexcept -> auto = default;
    auto               operator+=(const Wage other) noexcept -> Wage& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const Wage other) noexcept -> Wage& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) noexcept -> Wage& {
        value_ += other;
        return *this;
    }
    auto operator/=(const double other) noexcept -> Wage& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }

    [[nodiscard]] auto operator-() const noexcept -> Wage { return Wage{-value_}; }

    [[nodiscard]] explicit operator Money() const noexcept { return Money{value_}; }

  private:
    double value_;
};
[[nodiscard]] inline auto operator+(Wage lhs, const Wage rhs) noexcept -> Wage {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(Wage lhs, const Wage rhs) noexcept -> Wage {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(Wage lhs, const double rhs) noexcept -> Wage {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(const double lhs, const Wage rhs) noexcept -> Wage {
    return rhs * lhs;
}
[[nodiscard]] inline auto operator/(Wage lhs, const double rhs) noexcept -> Wage {
    lhs /= rhs;
    return lhs;
}

class [[nodiscard]] HeadCount final {
  public:
    [[nodiscard]] explicit HeadCount(const double value) noexcept : value_{value} {}
    [[nodiscard]] explicit HeadCount(const int value) noexcept
        : value_{static_cast<double>(value)} {}
    [[nodiscard]] explicit HeadCount(const std::size_t value) noexcept
        : value_{static_cast<double>(value)} {}
    [[nodiscard]] auto value() const noexcept -> double { return value_; }
    [[nodiscard]] auto operator<=>(const HeadCount&) const noexcept -> auto = default;
    auto               operator+=(const HeadCount other) noexcept -> HeadCount& {
        value_ += other.value();
        return *this;
    }
    auto operator-=(const HeadCount other) noexcept -> HeadCount& {
        value_ -= other.value();
        return *this;
    }
    auto operator*=(const double other) noexcept -> HeadCount& {
        value_ *= other;
        return *this;
    }
    auto operator/=(const double other) noexcept -> HeadCount& PRE(other != 0.0) {
        value_ /= other;
        return *this;
    }
    auto operator++() noexcept -> HeadCount& {
        ++value_;
        return *this;
    }
    auto operator--() noexcept -> HeadCount& {
        --value_;
        return *this;
    }

    [[nodiscard]] auto operator-() const noexcept -> HeadCount { return HeadCount{-value_}; }

  private:
    double value_;
};

[[nodiscard]] inline auto operator+(HeadCount lhs, const HeadCount rhs) noexcept -> HeadCount {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] inline auto operator-(HeadCount lhs, const HeadCount rhs) noexcept -> HeadCount {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(HeadCount lhs, const double rhs) noexcept -> HeadCount {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] inline auto operator*(const double lhs, const HeadCount rhs) noexcept -> HeadCount {
    return rhs * lhs;
}
[[nodiscard]] inline auto operator/(HeadCount lhs, const double rhs) noexcept -> HeadCount {
    lhs /= rhs;
    return lhs;
}

[[nodiscard]] inline auto operator*(Wage lhs, HeadCount rhs) noexcept -> Money {
    return Money{lhs.value() * rhs.value()};
}
[[nodiscard]] inline auto operator/(Money lhs, Wage rhs) noexcept -> HeadCount
    PRE(rhs != Wage{0.0}) {
    return HeadCount{lhs.value() / rhs.value()};
}
[[nodiscard]] inline auto operator/(Money lhs, HeadCount rhs) noexcept -> Wage
    PRE(rhs != HeadCount{0.0}) {
    return Wage{lhs.value() / rhs.value()};
}
}  // namespace abm