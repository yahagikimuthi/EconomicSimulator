#pragma once

#include <cassert>
#include <concepts>
#include <optional>
#include <tuple>
#include <type_traits>
#include <variant>

#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm {
template <typename F>
concept PayTaxFn = requires(F f, Money preTaxAmount) {
    { f(preTaxAmount) } -> std::same_as<Money>;
};

template <typename F>
concept TryWithdrawFn = requires(F f, Budget budget) {
    { f(budget) } -> std::same_as<Money>;
};

template <typename F>
concept DepositFn = requires(F f, Money deposit) {
    { f(deposit) } -> std::same_as<void>;
};

template <typename F>
concept AddGoodsFn = requires(F f, GoodsQuantity goods) {
    { f(goods) } -> std::same_as<void>;
};
}  // namespace abm

namespace abm::listener {



template <typename... Ts>
    requires(sizeof...(Ts) > 0UZ)
class Listener final {
  public:
    explicit Listener() noexcept = default;

    template <typename T>
        requires(std::is_same_v<T, Ts> or ...)
    void add(T& t) noexcept {
        assert(not std::get<std::optional<T&>>(listeners_));  // 再セットは禁止
        std::get<std::optional<T&>>(listeners_) = t;
    }

    template <typename F>
        requires(std::is_invocable_v<F, Ts&> and ...)
    void notify(F&& methodCaller) noexcept {
        auto callFunc = [&](auto& listener) noexcept -> void {
            if (listener) methodCaller(*listener);
        };
        std::apply(
            [&](auto&... listener) noexcept -> void { ((callFunc(listener)), ...); }, listeners_
        );
    }

  private:
    std::tuple<std::optional<Ts&>...> listeners_{};
};

template <typename T, typename Listener>
struct IsListenerImpl : std::false_type {};

template <typename T, typename... Ts>
struct IsListenerImpl<T, Listener<Ts...>> : std::bool_constant<(std::same_as<T, Ts> or ...)> {};

template <typename T, typename Listener>
concept IsListenerOrMonostate =
    IsListenerImpl<T, Listener>::value or std::same_as<T, std::monostate>;
}  // namespace abm::listener

namespace abm {
template <typename... Ts>
using Listener = listener::Listener<Ts...>;

template <typename T, typename Listener>
concept IsListenerOrMono = listener::IsListenerOrMonostate<T, Listener>;
}  // namespace abm