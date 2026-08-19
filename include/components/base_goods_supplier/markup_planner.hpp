#pragma once

#include <algorithm>
#include <limits>
#include <optional>

#include "components/base_goods_supplier/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"

namespace abm::base_goods::supplier {

// 前回の取引計画中、供給量が必要
// 前回の取引結果中、売上高が必要
class MarkupPlannerMemory {
  public:
    [[nodiscard]] explicit MarkupPlannerMemory(RandomGenerator& masterRng) noexcept;
    [[nodiscard]] auto rememberLastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supply_.log;
    }
    [[nodiscard]] auto rememberLastSalesAmount() const noexcept -> std::optional<GoodsQuantity> {
        return salesAmount_.log;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        salesAmount_.current = result.soldAmount;
    }
    void listenTradePlan(const TradePlan& plan) noexcept { supply_.current = plan.supply; }
    void clearLog() noexcept { supply_.clearLog(), salesAmount_.clearLog(); }
    void commit() noexcept { supply_.commit(), salesAmount_.commit(); }

  private:
    Memory<GoodsQuantity> supply_;
    Memory<GoodsQuantity> salesAmount_;
};

class MarkupPlanner final {
  public:
    [[nodiscard]] explicit MarkupPlanner(RandomGenerator& masterRng);

    [[nodiscard]] auto plan() noexcept -> double {
        const auto next = calcNextMarkup();
        memory_.clearLog();
        if (not next) return cache_.cache();
        cache_.memorize(*next);
        return *next;
    }

    void commit() noexcept {
        memory_.commit();
        cache_.commit();
    }

  private:
    // isSold = (前期供給 - 前期売上) / 前回供給 < 定数
    [[nodiscard]] auto calcNextMarkup() const noexcept -> std::optional<double> {
        const auto lastSupply      = memory_.rememberLastSupply();
        const auto lastSalesAmount = memory_.rememberLastSalesAmount();
        if (not lastSupply or not lastSalesAmount) return std::nullopt;
        ASSERT(*lastSupply >= GoodsQuantity{0.0});
        const auto inventory = *lastSupply - *lastSalesAmount;
        ASSERT(inventory >= GoodsQuantity{0.0});
        const auto isSupplied = *lastSupply == GoodsQuantity{0.0};
        const auto isSold     = isSupplied ? inventory / *lastSupply < targetInvRatio_ : true;
        return calcNextMarkup(isSold);
    }

    [[nodiscard]] auto calcNextMarkup(const bool isSold) const noexcept -> double {
        const auto alpha      = std::abs(rng_.randNormal(0.0, adjustVol_));
        const auto nextMarkup = cache_.cache() + (isSold ? alpha : -alpha);
        return markupGuard(nextMarkup);
    }

    [[nodiscard]] static auto markupGuard(const double markup) noexcept -> double {
        return std::max(markup, std::numeric_limits<double>::epsilon());
    }

    MarkupPlannerMemory     memory_;
    mutable RandomGenerator rng_;
    Cache<double>           cache_;
    const double            adjustVol_;
    const double            targetInvRatio_;
};
}  // namespace abm::base_goods::supplier