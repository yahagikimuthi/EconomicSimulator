#pragma once

#include <algorithm>
#include <concepts>

#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"

namespace abm {
template <typename T>
concept ValueObject =
    std::same_as<T, Money> or std::same_as<T, Price> or std::same_as<T, GoodsQuantity> or
    std::same_as<T, Wage> or std::same_as<T, HeadCount>;

template <ValueObject T>
[[nodiscard]] auto max(const T a, const T b) noexcept -> T {
    return T{std::max(a.value(), b.value())};
}
}  // namespace abm