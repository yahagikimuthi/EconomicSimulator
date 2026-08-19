#pragma once

#include "components/base_goods_supplier/trade_planner.hpp"
#include "components/production_goods_supplier/common.hpp"
#include "components/production_goods_supplier/trader.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::production_goods::supplier {
class TradingSystem final {
    using TradePlanner = base_goods::supplier::TradePlanner;
    using Market       = ProductionGoodsMarket;

  public:
    [[nodiscard]] explicit TradingSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng}, trader_{masterRng} {}

    void post(
        const AgentID id, const GoodsQuantity supply, const Money totalCost, Market& market
    ) noexcept {
        const auto plan = planner_.plan(supply, totalCost);
        trader_.post(id, plan, market);
    }

    void trade() noexcept { trader_.trade(); }

    void endStep(IMediator auto& mediator) {
        const auto result = trader_.publishTradeResult();
        mediator.publishTradeResult(result);
    }

    void reset() {
        planner_.reset();
        trader_.reset();
    }

  private:
    TradePlanner planner_;
    Trader       trader_;
};
}  // namespace abm::production_goods::supplier