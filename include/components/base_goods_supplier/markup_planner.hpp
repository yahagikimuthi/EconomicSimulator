#pragma once

#include <algorithm>
#include <limits>
#include <optional>
#include <pcg_random.hpp>

#include "components/base_goods_supplier/common.hpp"
#include "core/values/goods.hpp"
#include "setting.hpp"
#include "util.hpp"

namespace abm::base_goods::supplier {

// 前回の取引計画中、供給量が必要
// 前回の取引結果中、売上高が必要
class MarkupPlannerMemory final {
  public:
    [[nodiscard]] explicit MarkupPlannerMemory(RandomGenerator& masterRng) noexcept
        : supply_{GoodsQuantity{masterRng.random(setting::lastSupply)}},
          salesAmount_{GoodsQuantity{masterRng.random(setting::lastSalesAmount)}} {}
    [[nodiscard]] auto lastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supply_.log;
    }
    [[nodiscard]] auto lastSalesAmount() const noexcept -> std::optional<GoodsQuantity> {
        return salesAmount_.log;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        salesAmount_.next = result.soldAmount;
    }
    void listenTradePlan(const TradePlan& plan) noexcept { supply_.next = plan.supply; }
    void clearLog() noexcept { supply_.clearLog(), salesAmount_.clearLog(); }
    void reset() noexcept { supply_.reset(), salesAmount_.reset(); }

  private:
    Memory<GoodsQuantity> supply_;
    Memory<GoodsQuantity> salesAmount_;
};

class MarkupPlanner final {
  public:
    [[nodiscard]] explicit MarkupPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          cache_{masterRng.random(setting::lastMarkup)},
          rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::markupAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeTradePlan(memory_);
        mediator.subscribeTradeResult(memory_);
    }

    [[nodiscard]] auto plan(const double targetIvRatio) noexcept -> double {
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
    ) const noexcept -> std::optional<double> {
        const auto lastSupply      = memory_.lastSupply();
        const auto lastSalesAmount = memory_.lastSalesAmount();
        if (not lastSupply or not lastSalesAmount) return std::nullopt;
        ASSERT(*lastSupply >= GoodsQuantity{0.0});
        const auto inventory = *lastSupply - *lastSalesAmount;
        ASSERT(inventory >= GoodsQuantity{0.0});
        const auto isSupplied = *lastSupply == GoodsQuantity{0.0};
        const auto isSold     = isSupplied ? inventory / *lastSupply < targetInvRatio : true;
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
    Cache<double>           cache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::base_goods::supplier