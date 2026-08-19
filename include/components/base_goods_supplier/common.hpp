#pragma once

#include <concepts>
#include <optional>

#include "core/values/goods.hpp"

namespace abm::base_goods::supplier {
struct TradePlan final {
    const Price         price;
    const GoodsQuantity supply;
};

struct TradeResult final {
    const GoodsQuantity soldAmount;
    const GoodsQuantity unsoldAmount;
    const GoodsQuantity totalDemand;
    const Money         sales;
};

template <typename T>
class Memory final {
  public:
    [[nodiscard]] explicit Memory(T l) noexcept : log{l} {}

    void commit() noexcept {
        if (not next) return;
        log = next, next.reset();
    }
    void clearLog() noexcept { log.reset(); }

    std::optional<T> log;
    std::optional<T> next{std::nullopt};
};

template <typename T>
class Cache final {
  public:
    [[nodiscard]] explicit Cache(const T cache) noexcept : cache_{cache} {}
    [[nodiscard]] auto cache() const noexcept -> T { return cache_; }

    void commit() noexcept {
        if (not next_) return;
        cache_ = *next_;
        next_.reset();
    }
    void memorize(const T next) noexcept { next_ = next; }

  private:
    T                cache_;
    std::optional<T> next_{std::nullopt};
};

template <typename T>
concept IMediator = requires(T t, const TradePlan& plan, const TradeResult& result) {
    { t.publishTradePlan(plan) } -> std::same_as<void>;
    { t.publishTradeResult(result) } -> std::same_as<void>;
};
}  // namespace abm::base_goods::supplier