#include "strategies/goods_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <functional>

#include "config/init_setup.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace goods_demander {
namespace {
[[nodiscard]] auto calcBudget(const PurchaseView& view, const double availableAsset) -> double {
    // 使用可能資産×限界消費性向を家計が当期に使用する予算とする
    const double mpc{view.mpc()};
    assert(0.0 < mpc && mpc < 1.0 && "mpc is different range");
    return availableAsset * view.mpc();
}

[[nodiscard]] auto pickEntry(
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  sampleCnt = config::goods_demander::goodsSampleCnt
) -> world::GoodsEntry& {
    std::reference_wrapper<world::GoodsEntry> betterEntry =
        helper::discreteDistribution(entryBox, &world::GoodsEntry::supply_);

    if (sampleCnt <= 1) return betterEntry.get();

    for (int _{}; _ < sampleCnt - 1; ++_) {
        auto& sampleEntry{helper::discreteDistribution(entryBox, &world::GoodsEntry::supply_)};
        if (betterEntry.get().price_ > sampleEntry.price_) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace

void purchase(
    PurchaseView                               view,
    const double                               availableAsset,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  maxPurchaseFrequency,
    const int&                                 step
) {
    if (entryBox.empty() or (step % maxPurchaseFrequency != view.myPhase())) {
        view.isPosting(false);
        return;
    }
    const double budget{calcBudget(view, availableAsset)};
    if (budget <= 0.0) {
        view.isPosting(false);
        return;
    }
    view.isPosting(true);
    auto& pickedEntry = pickEntry(entryBox);
    assert(pickedEntry.price_ > 0.0 && "price is required > 0.0");
    view.entry(pickedEntry, pickedEntry.requestBox_.emplace_back(budget / pickedEntry.price_));
}
}  // namespace goods_demander