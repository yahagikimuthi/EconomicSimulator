#pragma once

#include <cmath>
#include <concepts>
#include "others/type.hpp"

namespace abm::value_object {
template <typename T>
concept ComputableObject = requires(T t) {
    { t.value() } -> std::same_as<f64>;
} and std::is_constructible_v<T, f64>;

template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto ceil(const T a) noexcept -> T {
    return T{std::ceil(a.value())};
}

template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto pow(const T x, const f64 y) noexcept -> T {
    return T{std::pow(x.value(), y)};
}
}  // namespace abm::value_object