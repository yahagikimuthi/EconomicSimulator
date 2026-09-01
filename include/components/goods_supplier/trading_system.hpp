#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/trade_planner.hpp"
#include "components/common.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/goods_supplier/trader.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::goods::supplier {
template <typename T>
concept IMediator = base_goods::supplier::IMediator<T>;

class TradingSystem final {
    using TradePlanner = base_goods::supplier::TradePlanner;
    using TradePlan    = base_goods::supplier::TradePlan;

  public:
    explicit TradingSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng}, trader_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    [[nodiscard]] auto requiresSupply() noexcept -> GoodsQuantity {
        return planner_.requiresSupply();
    }

    void plan(
        const GoodsQuantity supply, const Money totalCost, IMediator auto& mediator
    ) noexcept {
        ASSERT(supply.isZeroOrMore());
        const auto plan = planner_.planTrading(supply, totalCost, mediator);
        plan_.emplace(plan);
    }

    void post(const AgentID id, Market& market) noexcept {
        ASSERT(plan_);
        trader_.post(id, *plan_, market);
    }

    void trade(FirmFinance& finance) noexcept { trader_.trade(finance); }

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
    std::optional<TradePlan> plan_;
    TradePlanner             planner_;
    Trader                   trader_;
};
}  // namespace abm::goods::supplier