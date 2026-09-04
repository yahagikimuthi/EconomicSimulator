#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/trade_planner.hpp"
#include "components/base_goods_supplier/trader.hpp"
#include "components/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"

namespace abm::base_goods::supplier {

template <EMarket SupplyGoodsT>
class TradingSystem final {
    using MarketT = Market<SupplyGoodsT>;

  public:
    explicit TradingSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng}, trader_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    [[nodiscard]] auto requiresSupply() noexcept -> GoodsQuantity {
        return planner_.requiresSupply();
    }

    void plan(
        const GoodsQuantity supply, const Budget totalCost, IMediator auto& mediator
    ) noexcept {
        ASSERT(supply.isZeroOrMore());
        const auto plan = planner_.planTrading(supply, totalCost, mediator);
        plan_.emplace(plan);
    }

    void post(const AgentID id, MarketT& market) noexcept {
        ASSERT(plan_);
        trader_.post(id, *plan_, market);
    }

    template <DepositFn F>
    void trade(F&& depositFn) noexcept {
        trader_.trade(std::forward<F>(depositFn));
    }

    void endTrading(IMediator auto& mediator) noexcept {
        const auto result = trader_.publishTradeResult();
        mediator.publishTradeResult(result);
    }

    void reset() noexcept {
        planner_.reset();
        trader_.reset();
    }

  private:
    std::optional<TradePlan> plan_;
    TradePlanner             planner_;
    Trader<SupplyGoodsT>     trader_;
};
}  // namespace abm::base_goods::supplier