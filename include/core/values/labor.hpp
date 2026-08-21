#pragma once

#include <cmath>
#include <compare>

#include "core/values/common.hpp"

namespace abm {
class [[nodiscard]] Wage final {
  public:
    [[nodiscard]] explicit constexpr Wage(const double value) noexcept : value_{value} {}
    [[nodiscard]] constexpr auto value() const noexcept -> double { return value_; }
    [[nodiscard]] constexpr auto operator<=>(const Wage&) const noexcept -> auto = default;

    constexpr auto operator+=(const Wage other) noexcept -> Wage& {
        value_ += other.value();
        return *this;
    }
    constexpr auto operator-=(const Wage other) noexcept -> Wage& {
        value_ -= other.value();
        return *this;
    }
    constexpr auto operator*=(const double other) noexcept -> Wage& {
        value_ += other;
        return *this;
    }
    constexpr auto operator/=(const double other) noexcept -> Wage& {
        ASSERT(other != 0.0);
        value_ /= other;
        return *this;
    }

    [[nodiscard]] constexpr auto operator-() const noexcept -> Wage { return Wage{-value_}; }

    [[nodiscard]] explicit constexpr operator Money() const noexcept { return Money{value_}; }

  private:
    double value_;
};
[[nodiscard]] constexpr auto operator+(Wage lhs, const Wage rhs) noexcept -> Wage {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator-(Wage lhs, const Wage rhs) noexcept -> Wage {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(Wage lhs, const double rhs) noexcept -> Wage {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(const double lhs, const Wage rhs) noexcept -> Wage {
    return rhs * lhs;
}
[[nodiscard]] constexpr auto operator/(Wage lhs, const double rhs) noexcept -> Wage {
    ASSERT(rhs != 0.0);
    lhs /= rhs;
    return lhs;
}

class [[nodiscard]] HeadCount final {
  public:
    [[nodiscard]] explicit constexpr HeadCount(const double value) noexcept : value_{value} {}
    [[nodiscard]] explicit constexpr HeadCount(const int value) noexcept
        : value_{static_cast<double>(value)} {}
    [[nodiscard]] explicit constexpr HeadCount(const std::size_t value) noexcept
        : value_{static_cast<double>(value)} {}
    [[nodiscard]] constexpr auto value() const noexcept -> double { return value_; }

    constexpr auto ceil() noexcept -> HeadCount& {
        value_ = std::ceil(value_);
        return *this;
    }
    [[nodiscard]] auto operator<=>(const HeadCount&) const noexcept -> auto = default;

    constexpr auto operator+=(const HeadCount other) noexcept -> HeadCount& {
        value_ += other.value();
        return *this;
    }
    constexpr auto operator-=(const HeadCount other) noexcept -> HeadCount& {
        value_ -= other.value();
        return *this;
    }
    constexpr auto operator*=(const double other) noexcept -> HeadCount& {
        value_ *= other;
        return *this;
    }
    constexpr auto operator/=(const double other) noexcept -> HeadCount& {
        ASSERT(other != 0.0);
        value_ /= other;
        return *this;
    }
    constexpr auto operator++() noexcept -> HeadCount& {
        ++value_;
        return *this;
    }
    constexpr auto operator--() noexcept -> HeadCount& {
        --value_;
        return *this;
    }

    [[nodiscard]] constexpr auto operator-() const noexcept -> HeadCount {
        return HeadCount{-value_};
    }

  private:
    double value_;
};

[[nodiscard]] constexpr auto operator+(HeadCount lhs, const HeadCount rhs) noexcept -> HeadCount {
    lhs += rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator-(HeadCount lhs, const HeadCount rhs) noexcept -> HeadCount {
    lhs -= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(HeadCount lhs, const double rhs) noexcept -> HeadCount {
    lhs *= rhs;
    return lhs;
}
[[nodiscard]] constexpr auto operator*(const double lhs, const HeadCount rhs) noexcept
    -> HeadCount {
    return rhs * lhs;
}
[[nodiscard]] constexpr auto operator/(HeadCount lhs, const double rhs) noexcept -> HeadCount {
    ASSERT(rhs != 0.0);
    lhs /= rhs;
    return lhs;
}

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