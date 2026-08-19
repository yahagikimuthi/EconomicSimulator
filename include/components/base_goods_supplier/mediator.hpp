#pragma once

#include <array>
#include <optional>
#include <variant>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "components/base_goods_supplier/markup_planner.hpp"
#include "components/base_goods_supplier/produsing.hpp"

namespace abm::base_goods::supplier::mediator {
using TradePlanListener = std::variant<EmployPlannerMemory, MarkupPlannerMemory>;
using TradeResultListener =
    std::variant<DemandForecastManagerMemory, MarkupPlannerMemory, Producer>;

class Mediator final {
    template <typename T, int N>
    using OptArr = std::array<std::optional<T>, N>;

  public:
    [[nodiscard]] Mediator() noexcept = default;

    void tradePlanListeners(OptArr<TradePlanListener&, 2>& listeners) noexcept {
        tradePlanListeners_ = listeners;
    }
    void tradeResultListeners(OptArr<TradeResultListener&, 3>& listeners) noexcept {
        tradeResultListeners_ = listeners;
    }

    void publishTradePlan(const TradePlan& plan) noexcept {
        for (std::optional<TradePlanListener&> optListener : tradePlanListeners_) {
            if (not optListener) continue;
            std::visit(
                [&](auto&& listener) noexcept -> void { listener.listenTradePlan(plan); },
                *optListener
            );
        }
    }

    void publishTradeResult(const TradeResult& result) noexcept {
        for (std::optional<TradeResultListener&> optListener : tradeResultListeners_) {
            if (not optListener) continue;
            std::visit(
                [&](auto&& listener) noexcept -> void { listener.listenTradeResult(result); },
                *optListener
            );
        }
    }

  private:
    OptArr<TradePlanListener&, 2>   tradePlanListeners_;
    OptArr<TradeResultListener&, 3> tradeResultListeners_;
};
}  // namespace abm::base_goods::supplier::mediator

namespace abm::base_goods::supplier {
using Mediator = mediator::Mediator;
}