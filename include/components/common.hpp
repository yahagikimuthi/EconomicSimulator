#pragma once

#include <concepts>

#include "values/common.hpp"

namespace abm {

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