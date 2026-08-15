#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "components/base_goods_demander.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] ConsumerGoodsDemander final : public BaseGoodsDemander<Market::consumerGoods> {
  public:
    ConsumerGoodsDemander(
        const RandomGenerator                        rng,
        const base_goods::demander::BudgetCalculator budgetCalculator,
        const Step                                   myPhase
    )
        : BaseGoodsDemander<Market::consumerGoods>::BaseGoodsDemander(rng, budgetCalculator),
          myPhase_{myPhase} {}

    void request(
        const Money asset, const Step step, tbb::concurrent_vector<ConsumerGoodsEntry> entryBox
    ) {
        if (isPass(asset, step, entryBox)) return;
        const Money budget{budgetCalculator_.calcBudget(asset)};
        if (budget <= Money{0.0}) return;
        isPosting_        = true;
        auto& pickedEntry = pickEntry(entryBox);
        myRequest_        = pickedEntry.request(GoodsQuantity{budget / pickedEntry.price});
    }

    void afterTrade() {
        if (not isPosting_) return;
        purchasing_ += myRequest_->entry.price * myRequest_->tradeAmount;
    }

    void endStep() {
        myRequest_.reset();
        isPosting_  = false;
        purchasing_ = Money{0.0};
    }

    auto purchase() const -> Money POST(money : money >= Money{0.0}) { return purchasing_; }

  private:
    auto isPass(
        const Money                                      asset,
        const Step                                       step,
        const tbb::concurrent_vector<ConsumerGoodsEntry> entryBox
    ) const -> bool {
        if (asset <= Money{0.0}) return true;
        if (entryBox.empty()) return true;
        const Step dayOfWeek{step % config::goods_demander::maxPurchaseFrequency};
        return dayOfWeek != myPhase_;
    }

    auto pickEntry(
        tbb::concurrent_vector<ConsumerGoodsEntry> entryBox,
        const int sampleCnt = config::goods_demander::goodsSampleCnt
    ) const -> ConsumerGoodsEntry& {
        using Entry = ConsumerGoodsEntry;

        auto toDouble{[](const Entry& entry) -> double { return entry.supply.value(); }};
        std::reference_wrapper<Entry> betterEntry = rng_.discreteDistribution(entryBox, toDouble);

        if (sampleCnt <= 1) return betterEntry.get();

        for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
            auto& sampleEntry = rng_.discreteDistribution(entryBox, toDouble);
            if (betterEntry.get().price <= sampleEntry.price) continue;
            betterEntry = std::ref(sampleEntry);
        }
        return betterEntry.get();
    }

    Money      purchasing_{0.0};
    const Step myPhase_;
};
}  // namespace abm