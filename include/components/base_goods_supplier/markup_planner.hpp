#pragma once

#include <limits>
#include <optional>
#include <pcg_random.hpp>

#include "components/base_goods_supplier/common.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "values/goods.hpp"
#include "values/integrate.hpp"

namespace abm::base_goods::supplier {

// 前回の取引計画中、供給量が必要
// 前回の取引結果中、売上高が必要
class MarkupPlannerMemory final {
  public:
    [[nodiscard]] explicit constexpr MarkupPlannerMemory(RandomGenerator& masterRng) noexcept
        : supply_{GoodsQuantity{masterRng.random(setting::lastSupply)}},
          salesAmount_{GoodsQuantity{masterRng.random(setting::lastSalesAmount)}} {}
    [[nodiscard]] auto lastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supply_.log();
    }
    [[nodiscard]] auto lastSalesAmount() const noexcept -> std::optional<GoodsQuantity> {
        return salesAmount_.log();
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        ASSERT(result.soldAmount >= GoodsQuantity{0.0});
        salesAmount_.next(result.soldAmount);
    }

    void listenTradePlan(const TradePlan& plan) noexcept {
        ASSERT(plan.supply >= GoodsQuantity{0.0});
        supply_.next(plan.supply);
    }

    void clearLog() noexcept { supply_.clearLog(), salesAmount_.clearLog(); }
    void reset() noexcept { supply_.reset(), salesAmount_.reset(); }

  private:
    Memory<GoodsQuantity> supply_;
    Memory<GoodsQuantity> salesAmount_;
};

class MarkupPlanner final {
  public:
    [[nodiscard]] explicit constexpr MarkupPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          cache_{MarkupRate{masterRng.random(setting::lastMarkup)}},
          rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::markupAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeTradePlan(memory_);
        mediator.subscribeTradeResult(memory_);
    }

    [[nodiscard]] auto plan(const double targetIvRatio) noexcept -> MarkupRate {
        ASSERT(0.0 < targetIvRatio and targetIvRatio < 1.0);

        const auto next = calcNextMarkup(targetIvRatio);
        memory_.clearLog();
        if (not next) return cache_.cache();
        cache_.next(*next);
        return *next;
    }

    void reset() noexcept {
        memory_.reset();
        cache_.reset();
    }

  private:
    // isSold = (前期供給 - 前期売上) / 前回供給 < 定数
    [[nodiscard]] auto calcNextMarkup(const double targetInvRatio
    ) const noexcept -> std::optional<MarkupRate> {
        const auto lastSupply      = memory_.lastSupply();
        const auto lastSalesAmount = memory_.lastSalesAmount();
        if (not lastSupply or not lastSalesAmount) return std::nullopt;
        ASSERT(*lastSupply >= GoodsQuantity{0.0});
        const auto inventory  = *lastSupply - *lastSalesAmount;
        const auto isSupplied = *lastSupply != GoodsQuantity{0.0};
        const auto isSold     = isSupplied ? inventory / *lastSupply < targetInvRatio : true;
        return calcNextMarkup(isSold);
    }

    [[nodiscard]] auto calcNextMarkup(const bool isSold) const noexcept -> MarkupRate {
        const auto alpha      = std::abs(rng_.randNormal(0.0, adjustVol_));
        const auto nextMarkup = cache_.cache() + MarkupRate{(isSold ? alpha : -alpha)};
        return guard(nextMarkup);
    }

    [[nodiscard]] static auto guard(const MarkupRate markup) noexcept -> MarkupRate {
        return max(markup, MarkupRate{std::numeric_limits<double>::epsilon()});
    }

    MarkupPlannerMemory     memory_;
    Cache<MarkupRate>       cache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::base_goods::supplier