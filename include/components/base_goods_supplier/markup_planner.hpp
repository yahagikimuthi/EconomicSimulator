#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>

#include "components/base_goods_supplier/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier {

// 前回の取引計画中、供給量が必要
// 前回の取引結果中、売上高が必要
class MarkupPlannerMemory final {
  public:
    explicit MarkupPlannerMemory(RandomGenerator& masterRng) noexcept
        : supplyPlan_{GoodsQuantity{masterRng.random(setting::lastSupply)}},
          salesAmount_{GoodsQuantity{masterRng.random(setting::lastSalesAmount)}} {}
    [[nodiscard]] auto lastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supplyPlan_.log();
    }
    [[nodiscard]] auto lastSalesAmount() const noexcept -> std::optional<GoodsQuantity> {
        return salesAmount_.log();
    }

    // 供給量がゼロであるものは学習に悪影響であるからそもそも記憶しない
    void listenTradePlan(const TradePlan& plan) noexcept {
        assert(plan.supply.isZeroOrMore());
        if (plan.supply.isPositive()) supplyPlan_.next(plan.supply);
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        assert(result.soldAmount.isZeroOrMore());
        if (supplyPlan_.wasSetNext()) salesAmount_.next(result.soldAmount);
        assert(supplyPlan_.wasSetNext() == salesAmount_.wasSetNext());
        supplyPlan_.reset();
        salesAmount_.reset();
    }

    void clearLog() noexcept { supplyPlan_.clearLog(), salesAmount_.clearLog(); }

  private:
    Memory<GoodsQuantity> supplyPlan_;
    Memory<GoodsQuantity> salesAmount_;
};

class MarkupPlanner final {
  public:
    explicit MarkupPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          cache_{masterRng.random(setting::lastMarkup)},
          rng_{{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::markupAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeTradePlan(memory_);
        mediator.subscribeTradeResult(memory_);
    }

    [[nodiscard]] auto plan(const double targetIvRatio) noexcept -> MarkupRate {
        assert(0.0 < targetIvRatio and targetIvRatio < 1.0);

        const auto next = calcNextMarkup(targetIvRatio);
        memory_.clearLog();
        if (not next) return cache_;
        cache_ = *next;
        return *next;
    }

  private:
    // isSold = (前期供給 - 前期売上) / 前回供給 < 定数
    [[nodiscard]] auto calcNextMarkup(const double targetInvRatio
    ) const noexcept -> std::optional<MarkupRate> {
        const auto lastSupply      = memory_.lastSupply();
        const auto lastSalesAmount = memory_.lastSalesAmount();
        if (not lastSupply or not lastSalesAmount) return std::nullopt;
        assert(lastSupply->isZeroOrMore());
        assert(lastSalesAmount->isZeroOrMore());

        const auto unsold = *lastSupply - *lastSalesAmount;
        const auto isSold = unsold / *lastSupply < targetInvRatio;
        return calcNextMarkup(isSold);
    }

    [[nodiscard]] auto calcNextMarkup(const bool isSold) const noexcept -> MarkupRate {
        const auto alpha      = std::abs(rng_.randNormal(0.0, adjustVol_));
        const auto nextMarkup = cache_ + MarkupRate{(isSold ? alpha : -alpha)};
        return guard(nextMarkup);
    }

    [[nodiscard]] static auto guard(const MarkupRate markup) noexcept -> MarkupRate {
        return std::max(markup, MarkupRate{std::numeric_limits<double>::epsilon()});
    }

    MarkupPlannerMemory     memory_;
    MarkupRate              cache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::base_goods::supplier