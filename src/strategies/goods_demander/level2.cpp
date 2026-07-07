#include "strategies/goods_demander/level2.hpp"

#include <functional>

#include "helper.hpp"
#include "strategies/goods_demander/level1.hpp"
#include "world/message.hpp"

namespace goods_demander {
[[nodiscard]] auto calcBudget(const PurchaseView& view, const double availableAsset) -> double {
    return availableAsset * view.mpc();
}

[[nodiscard]] auto pickEntry(
    tbb::concurrent_vector<world::GoodsEntry>& entryBox, const int sampleCnt
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
}  // namespace goods_demander