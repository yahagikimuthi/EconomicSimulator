#pragma once

#include <optional>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/trade_planner.hpp"
#include "components/common.hpp"
#include "components/consumer_goods_supplier/common.hpp"
#include "components/consumer_goods_supplier/trader.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"

namespace abm::consumer_goods::supplier {
class TradingSystem final {
    using TradePlanner = base_goods::supplier::TradePlanner;
    using TradePlan    = base_goods::supplier::TradePlan;

  public:
    explicit TradingSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng}, trader_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    void plan(
        const GoodsQuantity supply, const Money totalCost, IMediator auto& mediator
    ) noexcept {
        const auto plan = planner_.planTrading(supply, totalCost, mediator);
        plan_           = plan;
        mediator.publishTradePlan(plan);
    }

    void post(
        const GoodsQuantity supply, const Money totalCost, Market& market, IMediator auto& mediator
    ) noexcept {
        const auto plan = planner_.planTrading(supply, totalCost, mediator);
        mediator.publishTradePlan(plan);
        trader_.post(plan, market);
    }

    void post([[maybe_unused]] const AgentID _, Market& market) noexcept {
        trader_.post(*plan_, market);
    }

    [[nodiscard]] auto requiresSupply() noexcept -> GoodsQuantity {
        return planner_.requiresSupply();
    }

    void trade() noexcept { trader_.trade(); }

    void endStep(AssetPlusFn auto&& assetPlus, IMediator auto& mediator) noexcept {
        const auto result = trader_.publishResult();
        if (not result) return;
        assetPlus(result->sales);
        mediator.publishTradeResult(*result);
    }

    void reset() noexcept {
        planner_.reset();
        trader_.reset();
    }

  private:
    TradePlanner             planner_;
    Trader                   trader_;
    std::optional<TradePlan> plan_{std::nullopt};
};
}  // namespace abm::consumer_goods::supplier