#pragma once

#include <concepts>

#include "values/common.hpp"

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

template <typename F>
concept TryWithdrawFn = requires(F f, Budget budget) {
    { f(budget) } -> std::same_as<Money>;
};

template <typename F>
concept DepositFn = requires(F f, Money deposit) {
    { f(deposit) } -> std::same_as<void>;
};
}  // namespace abm