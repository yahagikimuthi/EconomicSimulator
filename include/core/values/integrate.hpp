#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>

#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"

namespace abm::value_object {
template <typename T>
concept ComputableObject =
    (std::same_as<T, Money> or std::same_as<T, Price> or std::same_as<T, GoodsQuantity> or
     std::same_as<T, HeadCount> or std::same_as<T, Wage>) and
    requires(T t) {
        { t.value() } -> std::same_as<double>;
    };
}  // namespace abm::value_object

namespace abm {
template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto max(const T a, const T b) noexcept -> T {
    return T{std::max(a.value(), b.value())};
}

template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto min(const T a, const T b) noexcept -> T {
    return T{std::min(a.value(), b.value())};
}

template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto ceil(const T a) noexcept -> T {
    return T{std::ceil(a.value())};
}

template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto clamp(const T val, const T low, const T high) noexcept -> T {
    return std::clamp(val.value(), low.value(), high.value());
}
}  // namespace abm