#pragma once

#include <cassert>
#include <concepts>
#include <optional>
#include <utility>
#include <variant>

#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/drop_box.hpp"

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
    explicit Memory(const T log) noexcept : log_{log} {}

    [[nodiscard]] auto log() const noexcept -> std::optional<T> { return log_; }
    [[nodiscard]] auto wasSetNext() const noexcept -> bool { return next_.has_value(); }

    void reset() noexcept {
        if (next_) log_ = std::exchange(next_, std::nullopt);
    }
    void clearLog() noexcept { log_.reset(); }
    void next(const T next) noexcept { next_ = next; }

  private:
    std::optional<T> log_;
    std::optional<T> next_{std::nullopt};
};

class CentralMemory final {
  public:
    explicit CentralMemory() noexcept = default;

    void listenTradePlan(const TradePlan& plan) noexcept {
        assert(plan.price.isZeroOrMore());
        assert(plan.supply.isZeroOrMore());
        pricePlan_  = plan.price;
        supplyPlan_ = plan.supply;
    }

    void listenMarkupPlan(const MarkupRate rate) noexcept {
        assert(rate.isPositive());
        markupPlan_ = rate;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        if (result.sales.isPositive()) lastSales_ = result.sales;
    }

    void logging(BaseGoodsDropBox auto& dropBox) noexcept {
        if (pricePlan_) dropBox.prices.add(*pricePlan_);
        if (markupPlan_) dropBox.markups.add(*markupPlan_);
        if (supplyPlan_) dropBox.supplies.add(*supplyPlan_);
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

template <typename T>
concept IMediator = requires(
    T mediator, std::monostate listener, TradePlan plan, TradeResult result, MarkupRate markup
) {
    { mediator.subscribeTradePlan(listener) } noexcept -> std::same_as<void>;
    { mediator.subscribeMarkupPlan(listener) } noexcept -> std::same_as<void>;
    { mediator.subscribeTradeResult(listener) } noexcept -> std::same_as<void>;
    { mediator.publishTradePlan(plan) } noexcept -> std::same_as<void>;
    { mediator.publishMarkupPlan(markup) } noexcept -> std::same_as<void>;
    { mediator.publishTradeResult(result) } noexcept -> std::same_as<void>;
};
}  // namespace abm::base_goods::supplier