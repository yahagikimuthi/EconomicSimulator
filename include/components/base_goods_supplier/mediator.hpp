#pragma once

#include <array>
#include <optional>
#include <variant>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "components/base_goods_supplier/markup_planner.hpp"
#include "components/base_goods_supplier/produsing.hpp"

namespace abm::base_goods::supplier::mediator {
using TradePlanListener =
    std::variant<std::optional<EmployPlannerMemory&>, std::optional<MarkupPlannerMemory&>>;
using TradeResultListener = std::variant<
    std::optional<DemandForecastManagerMemory&>,
    std::optional<MarkupPlannerMemory&>,
    std::optional<Producer&>>;

class Mediator final {
  public:
    [[nodiscard]] Mediator() noexcept = default;

    template <typename T>
    void subscribeTradePlan(T& t) noexcept {
        auto& arr = tradePlanListeners_;
        if constexpr (std::is_same_v<T, EmployPlannerMemory>) {
            arr[0] = t;
        } else if constexpr (std::is_same_v<T, MarkupPlannerMemory>) {
            arr[1] = t;
        }
    }

    template <typename T>
    void subscribeTradeResult(T& t) noexcept {
        auto& arr = tradeResultListeners_;
        if constexpr (std::is_same_v<T, DemandForecastManagerMemory>) {
            arr[0] = t;
        } else if constexpr (std::is_same_v<T, MarkupPlannerMemory>) {
            arr[1] = t;
        } else if constexpr (std::is_same_v<T, Producer>) {
            arr[2] = t;
        }
    }

    void publishTradePlan(const TradePlan& plan) noexcept {
        for (auto opt : tradePlanListeners_) {
            std::visit(
                [&](auto&& listener) noexcept -> void {
                    if (not listener) return;
                    listener->listenTradePlan(plan);
                },
                opt
            );
        }
    }

    void publishTradeResult(const TradeResult& result) noexcept {
        for (auto opt : tradeResultListeners_) {
            std::visit(
                [&](auto&& listener) noexcept -> void {
                    if (not listener) return;
                    listener->listenTradeResult(result);
                },
                opt
            );
        }
    }

  private:
    std::array<TradePlanListener, 2>   tradePlanListeners_;
    std::array<TradeResultListener, 3> tradeResultListeners_;
};
}  // namespace abm::base_goods::supplier::mediator

namespace abm::base_goods::supplier {
using Mediator = mediator::Mediator;
}