#pragma once

#include <concepts>
#include <optional>
#include <variant>

#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "world/common.hpp"

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
    [[nodiscard]] constexpr explicit Memory(const T log) noexcept : log_{log} {}

    void reset() noexcept {
        if (not next_) return;
        log_ = next_, next_.reset();
    }
    void clearLog() noexcept { log_.reset(); }
    void next(const T next) noexcept { next_ = next; }

    [[nodiscard]] auto log() const noexcept -> std::optional<T> { return log_; }

  private:
    std::optional<T> log_;
    std::optional<T> next_{std::nullopt};
};

template <typename T>
class Cache final {
  public:
    [[nodiscard]] explicit constexpr Cache(const T cache) noexcept : cache_{cache} {}
    [[nodiscard]] auto cache() const noexcept -> T { return cache_; }

    void reset() noexcept {
        if (not next_) return;
        cache_ = *next_;
        next_.reset();
    }
    void next(const T next) noexcept { next_ = next; }

  private:
    T                cache_;
    std::optional<T> next_{std::nullopt};
};

class CentralMemory {
  public:
    [[nodiscard]] constexpr CentralMemory() noexcept = default;

    void listenTradePlan(const TradePlan& plan) noexcept {
        ASSERT(plan.price >= Price{0.0});
        ASSERT(plan.supply >= GoodsQuantity{0.0});
        pricePlan_  = plan.price;
        supplyPlan_ = plan.supply;
    }

    void listenMarkupPlan(const MarkupRate markup) noexcept {
        ASSERT(markup > MarkupRate{0.0});
        markupPlan_ = markup;
    }

    void listenTradeResult(const TradeResult& result) noexcept { lastSales_ = result.sales; }

    void logging(CensusDropBox& dropBox) noexcept {
        if (pricePlan_) dropBox.prices.emplace_back(pricePlan_->value());
        if (markupPlan_) dropBox.markups.emplace_back(markupPlan_->value());
        if (supplyPlan_) dropBox.supplies.emplace_back(supplyPlan_->value());
        pricePlan_.reset();
        markupPlan_.reset();
        supplyPlan_.reset();
    }

    [[nodiscard]] auto lastSales() const noexcept -> Money { return lastSales_; }

  private:
    std::optional<Price>         pricePlan_{std::nullopt};
    std::optional<MarkupRate>    markupPlan_{std::nullopt};
    std::optional<GoodsQuantity> supplyPlan_{std::nullopt};
    Money                        lastSales_{0.0};
};

template <typename T, typename U = std::monostate>
concept IMediator =
    requires(T t, U u, const TradePlan& plan, MarkupRate markupPlan, const TradeResult& result) {
        { t.publishTradePlan(plan) } -> std::same_as<void>;
        { t.publishMarkupPlan(markupPlan) } -> std::same_as<void>;
        { t.publishTradeResult(result) } -> std::same_as<void>;
    };
}  // namespace abm::base_goods::supplier