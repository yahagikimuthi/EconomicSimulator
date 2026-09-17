#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/markup_planner.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/base_goods_supplier/trade_planner.hpp"
#include "components/common.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier {
class Mediator final {
    using PlanListener   = Listener<CentralMemory, MarkupPlannerMemory>;
    using MarkupListener = Listener<CentralMemory>;
    using ResultListener =
        Listener<CentralMemory, ProducingSystem, MarkupPlannerMemory, DemandForecastManagerMemory>;

  public:
    explicit Mediator() noexcept = default;

    void subscribeTradePlan(IsListenerOrMono<PlanListener> auto& listener) noexcept {
        tradePlanListeners_.add(listener);
    }

    void subscribeMarkupPlan(IsListenerOrMono<MarkupListener> auto& listener) noexcept {
        markupPlanListeners_.add(listener);
    }

    void subscribeTradeResult(IsListenerOrMono<ResultListener> auto& listener) noexcept {
        tradeResultListeners_.add(listener);
    }

    void publishTradePlan(const TradePlan& plan) noexcept {
        tradePlanListeners_.notify([&](auto& listener) noexcept -> void {
            listener.listenTradePlan(plan);
        });
    }

    void publishMarkupPlan(const MarkupRate markupPlan) noexcept {
        markupPlanListeners_.notify([markupPlan](auto& listener) noexcept -> void {
            listener.listenMarkupPlan(markupPlan);
        });
    }

    void publishTradeResult(const TradeResult& result) noexcept {
        tradeResultListeners_.notify([&](auto& listener) noexcept -> void {
            listener.listenTradeResult(result);
        });
    }

  private:
    PlanListener   tradePlanListeners_;
    MarkupListener markupPlanListeners_;
    ResultListener tradeResultListeners_;
};
}  // namespace abm::base_goods::supplier