#include "strategies/goods_demander/request.hpp"

#include <oneapi/tbb/concurrent_vector.h>
#include <tbb/concurrent_vector.h>
#include <cassert>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "config.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace goods_demander {
namespace {
[[nodiscard]] auto isPass(
    const tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                        step,
    const int                                        myPhase,
    const int maxFrequency = config::goods_demander::maxPurchaseFrequency
) -> bool {
    const bool isEmpty{entryBox.empty()};
    const int  dayOfWeek{step % maxFrequency};
    const bool isNotMyPhase{dayOfWeek != myPhase};
    return isEmpty or isNotMyPhase;
}

[[nodiscard]] auto calcBudget(const double mpc, const double availableAsset) -> double {
    // 使用可能資産×限界消費性向を家計が当期に使用する予算とする
    assert(0.0 < mpc && mpc < 1.0 && "mpc is different range");
    return availableAsset * mpc;
}

[[nodiscard]] auto pickEntry(
    pcg32&                                     rng,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  sampleCnt = config::goods_demander::goodsSampleCnt
) -> world::GoodsEntry& {
    std::reference_wrapper<world::GoodsEntry> betterEntry =
        helper::discreteDistribution(entryBox, rng, &world::GoodsEntry::supply_);

    if (sampleCnt <= 1) return betterEntry.get();

    for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
        auto& sampleEntry =
            helper::discreteDistribution(entryBox, rng, &world::GoodsEntry::supply_);
        if (betterEntry.get().price_ <= sampleEntry.price_) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace

void purchase(
    PurchaseView                               view,
    const double                               availableAsset,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  step
) {
    if (isPass(entryBox, step, view.myPhase())) {
        view.isPosting(false);
        return;
    }
    const double budget{calcBudget(view.mpc(), availableAsset - view.purchasing())};
    if (budget <= 0.0) {
        view.isPosting(false);
        return;
    }
    view.isPosting(true);
    auto& pickedEntry = pickEntry(view.rng(), entryBox);
    assert(pickedEntry.price_ > 0.0 && "price is required > 0.0");
    view.entry(pickedEntry.requestBox_.emplace_back(budget / pickedEntry.price_, pickedEntry));
}
}  // namespace goods_demander