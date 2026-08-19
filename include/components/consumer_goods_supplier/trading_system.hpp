#pragma once

#include "components/base_goods_supplier/trade_planner.hpp"
#include "components/consumer_goods_supplier/common.hpp"
#include "components/consumer_goods_supplier/trader.hpp"
#include "core/values/goods.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods::supplier {
class TradingSystem {
    using TradePlanner = base_goods::supplier::TradePlanner;
    using Market       = ConsumerGoodsMarket;

  public:
    [[nodiscard]] explicit TradingSystem() noexcept;

    void post(const GoodsQuantity supply, const Money totalCost, Market& market) noexcept {
        const auto plan = planner_.plan(supply, totalCost);
        trader_.post(plan, market);
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
}  // namespace abm::consumer_goods::supplier