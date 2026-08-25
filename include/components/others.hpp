#pragma once

#include <concepts>

#include "core/values/common.hpp"

namespace abm {
template <typename F>
concept AssetPlusFn = requires(F f, const Money money) {
    { f(money) } -> std::same_as<void>;
};

template <typename F>
concept AssetMinusFn = AssetPlusFn<F>;

template <typename F>
concept AfterTaxCalculatorFn = requires(F f, const Money profit) {
    { f(profit) } -> std::same_as<Money>;
};
}  // namespace abm