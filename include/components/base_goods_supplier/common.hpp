#pragma once

#include <concepts>
#include <optional>

#include "core/values/goods.hpp"
#include "util.hpp"

namespace abm::base_goods::supplier {
struct TradePlan {
    Price         price;
    GoodsQuantity supply;
};

struct TradeResult {
    GoodsQuantity soldAmount;
    GoodsQuantity unsoldAmount;
    GoodsQuantity totalDemand;
    Money         sales;
};

template <typename T>
class Memory final {
  public:
    [[nodiscard]] explicit Memory(RandomGenerator& masterRng) noexcept;

    void commit() noexcept {
        if (not current) return;
        log = current, current.reset();
    }
    void clearLog() noexcept { log.reset(); }

    std::optional<T> log;
    std::optional<T> current{std::nullopt};
};

template <typename T>
class Cache final {
  public:
    [[nodiscard]] explicit Cache(RandomGenerator& masterRng) noexcept;
    [[nodiscard]] auto cache() const noexcept -> T { return cache_; }

    void commit() noexcept {
        if (not next_) return;
        cache_ = *next_;
        next_.reset();
    }
    void memorize(const T next) noexcept { next_ = next; }

  private:
    T                cache_;
    std::optional<T> next_;
};

template <typename T>
concept IMediator = requires(T t, const TradePlan& plan, const TradeResult& result) {
    { t.publishTradePlan(plan) } -> std::same_as<void>;
    { t.publishTradeResult(result) } -> std::same_as<void>;
};
}  // namespace abm::base_goods::supplier