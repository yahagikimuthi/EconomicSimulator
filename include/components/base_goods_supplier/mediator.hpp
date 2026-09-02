#pragma once

#include <optional>
#include <tuple>
#include <type_traits>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "components/base_goods_supplier/markup_planner.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/base_goods_supplier/trade_planner.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier::mediator {
template <typename... Ts>
    requires(sizeof...(Ts) > 0UZ)
class Listener final {
  public:
    Listener() noexcept = default;

    template <typename T>
        requires(std::is_same_v<T, Ts> or ...)
    void add(T& t) noexcept {
        ASSERT(not std::get<std::optional<T&>>(listeners_));
        std::get<std::optional<T&>>(listeners_) = t;
    }

    template <typename F>
        requires(std::is_invocable_v<F, Ts> and ...)
    void notice(F&& methodCaller) noexcept {
        template for (auto& opt : listeners_) {
            if (opt) {
                methodCaller(*opt);
            }
        }
    }

  private:
    std::tuple<std::optional<Ts&>...> listeners_;
};

class Mediator final {
    using TradePlanListener  = Listener<EmployPlannerMemory, MarkupPlannerMemory, CentralMemory>;
    using MarkupPlanListener = Listener<CentralMemory>;
    using TradeResultListener =
        Listener<DemandForecastManagerMemory, MarkupPlannerMemory, ProducingSystem, CentralMemory>;

  public:
    Mediator() noexcept = default;

    template <typename T>
    void subscribeTradePlan(T& t) noexcept {
        tradePlanListeners_.add(t);
    }

    template <typename T>
    void subscribeMarkupPlan(T& t) noexcept {
        markupPlanListeners_.add(t);
    }

    template <typename T>
    void subscribeTradeResult(T& t) noexcept {
        tradeResultListeners_.add(t);
    }

    void publishTradePlan(const TradePlan& plan) noexcept {
        tradePlanListeners_.notice([&](auto&& listener) noexcept -> void {
            listener.listenTradePlan(plan);
        });
    }

    void publishMarkupPlan(const MarkupRate markupPlan) noexcept {
        markupPlanListeners_.notice([markupPlan](auto&& listener) noexcept -> void {
            listener.listenMarkupPlan(markupPlan);
        });
    }

    void publishTradeResult(const TradeResult& result) noexcept {
        tradeResultListeners_.notice([&](auto&& listener) noexcept -> void {
            listener.listenTradeResult(result);
        });
    }

  private:
    TradePlanListener   tradePlanListeners_;
    MarkupPlanListener  markupPlanListeners_;
    TradeResultListener tradeResultListeners_;
};
}  // namespace abm::base_goods::supplier::mediator

namespace abm::base_goods::supplier {
using Mediator = mediator::Mediator;
}