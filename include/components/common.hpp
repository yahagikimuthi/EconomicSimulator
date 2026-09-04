#pragma once

#include <concepts>
#include <optional>
#include <tuple>
#include <type_traits>

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

template <typename... Ts>
    requires(sizeof...(Ts) > 0UZ)
class Listener final {
  public:
    explicit Listener() noexcept = default;

    template <typename T>
        requires(std::is_same_v<T, Ts> or ...)
    void add(T& t) noexcept {
        ASSERT(not std::get<std::optional<T&>>(listeners_));
        std::get<std::optional<T&>>(listeners_) = t;
    }

    template <typename F>
        requires(std::is_invocable_v<F, Ts> and ...)
    void notice(F&& methodCaller) noexcept {
        auto callFunc = [&](auto& listener) noexcept -> void {
            if (listener) {
                methodCaller(*listener);
            }
        };
        std::apply(
            [&](auto&... listener) noexcept -> void { ((callFunc(listener)), ...); }, listeners_
        );
    }

  private:
    std::tuple<std::optional<Ts&>...> listeners_;
};
}  // namespace abm