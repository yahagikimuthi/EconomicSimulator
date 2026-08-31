#pragma once

#include <algorithm>
#include <limits>
#include <pcg_random.hpp>

#include "components/base_supplier/common.hpp"
#include "components/base_supplier/markup_planner.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier {
class PricePlanner final {
  public:
    explicit PricePlanner(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::priceAdjustVol)} {}

    [[nodiscard]] auto plan(
        const GoodsQuantity supply, const MarkupRate markup, const Money totalCost
    ) const noexcept -> Price {
        ASSERT(supply.isZeroOrMore());
        ASSERT(markup > MarkupRate{0.0});
        ASSERT(totalCost >= Money{0.0});

        const auto price = calcPrice(supply, markup, totalCost);
        const auto alpha = rng_.randNormal(0.0, adjustVol_, -1.0, 1.0);
        return guard(price * (1.0 + alpha));
    }

  private:
    [[nodiscard]] static auto calcPrice(
        const GoodsQuantity supply, const MarkupRate markup, const Money totalCost
    ) noexcept -> Price {
        const auto avgCost =
            Money{(supply != GoodsQuantity{0.0}) ? totalCost.value() / supply.value() : 0.0};
        const auto price = avgCost * (MarkupRate{1.0} + markup);
        return price;
    }

    [[nodiscard]] static auto guard(const Price price) noexcept -> Price {
        return std::max(price, Price{std::numeric_limits<double>::epsilon()});
    }

    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

// 前回の取引結果中、需要量が必要
class DemandForecastManagerMemory final {
  public:
    explicit DemandForecastManagerMemory(RandomGenerator& masterRng) noexcept
        : totalDemand_{GoodsQuantity{masterRng.random(setting::lastDemand)}} {}

    [[nodiscard]] auto lastTotalDemand() const noexcept -> std::optional<GoodsQuantity> {
        return totalDemand_.log();
    }
    void clearLog() noexcept { totalDemand_.clearLog(); }
    void reset() noexcept { totalDemand_.reset(); }
    void listenTradeResult(const TradeResult& result) noexcept {
        ASSERT(result.totalDemand.isZeroOrMore());
        totalDemand_.next(result.totalDemand);
    }

  private:
    Memory<GoodsQuantity> totalDemand_;
};

class DemandForecastManager final {
  public:
    explicit DemandForecastManager(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          cache_{GoodsQuantity{masterRng.random(setting::demandForecast)}},
          adjustment_{masterRng.random(setting::demandForecastAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeTradeResult(memory_);
    }
    [[nodiscard]] auto plan() noexcept -> GoodsQuantity {
        const auto next = calcNext();
        memory_.clearLog();
        if (not next) return cache_.cache();
        cache_.next(*next);

        ASSERT(next->isZeroOrMore());
        return *next;
    }

    void reset() noexcept {
        memory_.reset();
        cache_.reset();
    }

  private:
    [[nodiscard]] auto calcNext() const noexcept -> std::optional<GoodsQuantity> {
        const auto lastTotalDemand = memory_.lastTotalDemand();
        if (not lastTotalDemand) return std::nullopt;
        const auto lastForecast = cache_.cache();
        const auto out          = lastForecast + (adjustment_ * (*lastTotalDemand - lastForecast));
        return guard(out);
    }

    [[nodiscard]] static auto guard(const GoodsQuantity expect) noexcept -> GoodsQuantity {
        return std::max(expect, GoodsQuantity{std::numeric_limits<double>::epsilon()});
    }

    DemandForecastManagerMemory memory_;
    Cache<GoodsQuantity>        cache_;
    const double                adjustment_;
};

class TradePlanner final {
  public:
    explicit TradePlanner(RandomGenerator& masterRng) noexcept
        : markupPlanner_{masterRng},
          pricePlanner_{masterRng},
          demandForecast_{masterRng},
          targetInvRatio_{masterRng.random(setting::targetInvRatio)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        markupPlanner_.acceptMediator(mediator);
        demandForecast_.acceptMediator(mediator);
    }

    [[nodiscard]] auto planTrading(
        const GoodsQuantity supply, const Money totalCost, IMediator auto& mediator
    ) noexcept -> TradePlan {
        const auto markup = markupPlanner_.plan(targetInvRatio_);
        mediator.publishMarkupPlan(markup);
        const auto price = pricePlanner_.plan(supply, markup, totalCost);
        return {.price = price, .supply = supply};
    }

    [[nodiscard]] auto requiresSupply() noexcept -> GoodsQuantity {
        const auto demandForecast = demandForecast_.plan();
        return demandForecast / (1.0 - targetInvRatio_);
    }

    void reset() noexcept {
        markupPlanner_.reset();
        demandForecast_.reset();
    }

  private:
    MarkupPlanner         markupPlanner_;
    PricePlanner          pricePlanner_;
    DemandForecastManager demandForecast_;
    const double          targetInvRatio_;
};
}  // namespace abm::base_goods::supplier