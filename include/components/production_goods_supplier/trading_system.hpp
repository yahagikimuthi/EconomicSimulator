#pragma once

#include "components/base_goods_supplier/trade_planner.hpp"
#include "components/common.hpp"
#include "components/production_goods_supplier/common.hpp"
#include "components/production_goods_supplier/trader.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"

namespace abm::production_goods::supplier {
class TradingSystem final {
    using TradePlanner = base_goods::supplier::TradePlanner;

  public:
    [[nodiscard]] explicit constexpr TradingSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng}, trader_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    [[nodiscard]] auto requiresSupply() noexcept -> GoodsQuantity {
        return planner_.requiresSupply();
    }

    void post(
        const AgentID       id,
        const GoodsQuantity supply,
        const Money         totalCost,
        Market&             market,
        IMediator auto&     mediator
    ) noexcept {
        ASSERT(supply >= GoodsQuantity{0.0});
        const auto plan = planner_.planTrading(supply, totalCost, mediator);
        trader_.post(id, plan, market);
    }

    void trade() noexcept { trader_.trade(); }

    void endStep(AssetPlusFn auto&& assetPlus, IMediator auto& mediator) noexcept {
        const auto result = trader_.publishTradeResult();
        if (not result) return;
        assetPlus(result->sales);
        mediator.publishTradeResult(*result);
    }

    void reset() noexcept {
        planner_.reset();
        trader_.reset();
    }

  private:
    TradePlanner planner_;
    Trader       trader_;
};
}  // namespace abm::production_goods::supplier