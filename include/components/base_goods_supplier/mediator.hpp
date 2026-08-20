#pragma once

#include <array>
#include <optional>
#include <variant>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "components/base_goods_supplier/markup_planner.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/base_goods_supplier/trade_planner.hpp"

namespace abm::base_goods::supplier::mediator {
template <typename T>
using Opt = std::optional<T>;

using TradePlanListener =
    std::variant<Opt<EmployPlannerMemory&>, Opt<MarkupPlannerMemory&>, Opt<CentralMemory&>>;
using MarkupPlanListener = std::variant<Opt<CentralMemory&>>;
using TradeResultListener =
    std::variant<Opt<DemandForecastManagerMemory&>, Opt<MarkupPlannerMemory&>, Opt<Producer&>>;

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
        } else if constexpr (std::is_same_v<T, CentralMemory>) {
            arr[2] = t;
        } else {
            static_assert(false);
        }
    }

    template <typename T>
    void subscribeMarkupPlan(T& t) noexcept {
        auto& arr = markupPlanListeners_;
        if constexpr (std::is_same_v<T, CentralMemory>) {
            arr[0] = t;
        } else {
            static_assert(false);
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
        } else {
            static_assert(false);
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

    void publishMarkupPlan(const double markup) noexcept {
        for (auto opt : markupPlanListeners_) {
            std::visit(
                [markup](auto&& listener) noexcept -> void {
                    if (not listener) return;
                    listener->listenMarkupPlan(markup);
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
    std::array<TradePlanListener, 3>   tradePlanListeners_;
    std::array<MarkupPlanListener, 1>  markupPlanListeners_;
    std::array<TradeResultListener, 3> tradeResultListeners_;
};
}  // namespace abm::base_goods::supplier::mediator

namespace abm::base_goods::supplier {
using Mediator = mediator::Mediator;
}