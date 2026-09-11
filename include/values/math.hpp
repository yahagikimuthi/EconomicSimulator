#pragma once

#include <cmath>
#include <concepts>

namespace abm::value_object {
template <typename T>
concept ComputableObject = requires(T t) {
    { t.value() } -> std::same_as<double>;
};
}  // namespace abm::value_object

namespace abm {
template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto ceil(const T a) noexcept -> T {
    return T{std::ceil(a.value())};
}

template <value_object::ComputableObject T>
[[nodiscard]] constexpr auto pow(const T x, const double y) noexcept -> T {
    return T{std::pow(x.value(), y)};
}
}  // namespace abm