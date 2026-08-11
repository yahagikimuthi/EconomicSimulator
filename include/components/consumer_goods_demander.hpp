#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace consumer_goods::demander::internal {
[[nodiscard]] inline auto pickEntry(
    pcg32&                                             rng,
    tbb::concurrent_vector<world::ConsumerGoodsEntry>& entryBox,
    const int sampleCnt = config::goods_demander::goodsSampleCnt
) -> world::ConsumerGoodsEntry& {
    auto toDouble{[](const world::ConsumerGoodsEntry& entry) -> double {
        return entry.supply.value();
    }};
    std::reference_wrapper<world::ConsumerGoodsEntry> betterEntry =
        helper::discreteDistribution(entryBox, rng, toDouble);

    if (sampleCnt <= 1) return betterEntry.get();

    for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
        auto& sampleEntry = helper::discreteDistribution(entryBox, rng, toDouble);
        if (betterEntry.get().price <= sampleEntry.price) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace consumer_goods::demander::internal

namespace consumer_goods::demander {
class [[nodiscard]] ConsumerGoodsDemander {
  public:
    ConsumerGoodsDemander(const pcg32 rng, const double mpc, const Step myPhase)
        : rng_{rng}, mpc_{mpc}, myPhase_{myPhase} {}

    void request(
        const Money                                        asset,
        const Step                                         step,
        tbb::concurrent_vector<world::ConsumerGoodsEntry>& entryBox
    ) {
        if (isPass(asset, step, entryBox)) return;
        const Money budget{calcBudget(asset)};
        if (budget <= Money{0.0}) return;
        isPosting_        = true;
        auto& pickedEntry = internal::pickEntry(rng_, entryBox);
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
        const Money                                              asset,
        const Step                                               step,
        const tbb::concurrent_vector<world::ConsumerGoodsEntry>& entryBox
    ) const -> bool {
        if (asset <= Money{0.0}) return true;
        if (entryBox.empty()) return true;
        const Step dayOfWeek{step % config::goods_demander::maxPurchaseFrequency};
        return dayOfWeek != myPhase_;
    }

    auto calcBudget(const Money asset) const -> Money { return asset * mpc_; }

    pcg32                                             rng_;
    std::optional<const world::ConsumerGoodsRequest&> myRequest_{std::nullopt};
    bool                                              isPosting_{false};
    Money                                             purchasing_{0.0};
    const double                                      mpc_;
    const Step                                        myPhase_;
};
}  // namespace consumer_goods::demander