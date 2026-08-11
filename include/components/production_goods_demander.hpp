#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace production_goods::demander::internal {
[[nodiscard]] inline auto pickEntry(
    pcg32&                                               rng,
    tbb::concurrent_vector<world::ProductionGoodsEntry>& entryBox,
    const int sampleCnt = config::goods_demander::goodsSampleCnt
) -> world::ProductionGoodsEntry& {
    auto toDouble{[](const world::ProductionGoodsEntry& entry) -> double {
        return entry.supply.value();
    }};
    std::reference_wrapper<world::ProductionGoodsEntry> betterEntry =
        helper::discreteDistribution(entryBox, rng, toDouble);

    if (sampleCnt <= 1) return betterEntry.get();

    for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
        auto& sampleEntry = helper::discreteDistribution(entryBox, rng, toDouble);
        if (betterEntry.get().price <= sampleEntry.price) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace production_goods::demander::internal

namespace production_goods::demander {
class [[nodiscard]] ProductionGoodsDemander {
  public:
    ProductionGoodsDemander(const pcg32 rng, const double mpc) : rng_{rng}, mpc_{mpc} {}

    void request(
        const GoodsQuantity                                  desiredAmount,
        const Money                                          asset,
        tbb::concurrent_vector<world::ProductionGoodsEntry>& entryBox
    ) {
        if (isPass(asset, entryBox)) return;
        const Money budget{calcBudget(asset)};
        if (budget <= Money{0.0}) return;
        isPosting_                      = true;
        auto&               pickedEntry = internal::pickEntry(rng_, entryBox);
        const GoodsQuantity purchaseAmount{
            std::min(desiredAmount.value(), (budget / pickedEntry.price).value())
        };
        myRequest_ = pickedEntry.request(purchaseAmount);
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
        const Money asset, const tbb::concurrent_vector<world::ProductionGoodsEntry>& entryBox
    ) const -> bool {
        if (asset <= Money{0.0}) return true;
        if (entryBox.empty()) return true;
        return false;
    }

    auto calcBudget(const Money asset) const -> Money { return asset * mpc_; }

    pcg32                                               rng_;
    std::optional<const world::ProductionGoodsRequest&> myRequest_{std::nullopt};
    bool                                                isPosting_{false};
    Money                                               purchasing_{0.0};
    const double                                        mpc_;
};
}  // namespace production_goods::demander