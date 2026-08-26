#pragma once

#include <optional>
#include <tuple>
#include <type_traits>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "components/base_goods_supplier/markup_planner.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/base_goods_supplier/trade_planner.hpp"
#include "core/values/goods.hpp"

namespace abm::base_goods::supplier::mediator {
template <typename... Ts>
class Listener {
    static_assert(sizeof...(Ts) >= 1UZ);

  public:
    [[nodiscard]] Listener() noexcept = default;

    template <typename T>
        requires std::disjunction_v<std::is_same<T, Ts>...>
    void add(T& t) noexcept {
        std::get<std::optional<T&>>(listeners_) = t;
    }

    template <typename F>
        requires std::conjunction_v<std::is_invocable<F, Ts>...>
    void publish(F methodCaller) noexcept {
        auto notice = [&](auto&& opt) -> void {
            if (opt) {
                methodCaller(*opt);
            }
        };
        std::apply([&](auto&&... opts) -> void { ((notice(opts)), ...); }, listeners_);
    }

  private:
    std::tuple<std::optional<Ts&>...> listeners_;
};

class Mediator {
    using TradePlanListener  = Listener<EmployPlannerMemory, MarkupPlannerMemory, CentralMemory>;
    using MarkupPlanListener = Listener<CentralMemory>;
    using TradeResultListener =
        Listener<DemandForecastManagerMemory, MarkupPlannerMemory, ProducingSystem, CentralMemory>;

  public:
    [[nodiscard]] constexpr Mediator() noexcept = default;

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
        tradePlanListeners_.publish([&](auto&& listener) -> void {
            listener.listenTradePlan(plan);
        });
    }

    void publishMarkupPlan(const MarkupRate markupPlan) noexcept {
        markupPlanListeners_.publish([markupPlan](auto&& listener) -> void {
            listener.listenMarkupPlan(markupPlan);
        });
    }

    void publishTradeResult(const TradeResult& result) noexcept {
        tradeResultListeners_.publish([&](auto&& listener) -> void {
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