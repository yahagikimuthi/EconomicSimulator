#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>

#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "core/values/mixin.hpp"

namespace abm::value_object {
template <typename T>
concept ComputableObject = (std::derived_from<T, BaseValueObject<double>>) and requires(T t) {
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
[[nodiscard]] constexpr auto clamp(T val, T low, T high) noexcept -> T {
    return T{std::clamp(val.value(), low.value(), high.value())};
}
}  // namespace abm