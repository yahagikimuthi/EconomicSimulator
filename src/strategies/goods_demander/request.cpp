#include "components/goods_demander.hpp"

#include <oneapi/tbb/concurrent_vector.h>
#include <tbb/concurrent_vector.h>
#include <cassert>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "config.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace {
[[nodiscard]] auto pickEntry(
    pcg32&                                     rng,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  sampleCnt = config::goods_demander::goodsSampleCnt
) -> world::GoodsEntry& {
    std::reference_wrapper<world::GoodsEntry> betterEntry =
        helper::discreteDistribution(entryBox, rng, &world::GoodsEntry::supply);

    if (sampleCnt <= 1) return betterEntry.get();

    for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
        auto& sampleEntry = helper::discreteDistribution(entryBox, rng, &world::GoodsEntry::supply);
        if (betterEntry.get().price <= sampleEntry.price) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace

namespace goods_demander {

auto GoodsDemander::isPass(
    const double asset, const int step, const tbb::concurrent_vector<world::GoodsEntry>& entryBox
) const -> bool {
    if (asset <= 0.0) return true;
    if (entryBox.empty()) return true;
    const int dayOfWeek{step % config::goods_demander::maxPurchaseFrequency};
    return dayOfWeek != myPhase_;
}

void GoodsDemander::request(
    const double asset, const int step, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    if (isPass(asset, step, entryBox)) return;
    const double budget{calcBudget(asset)};
    if (budget <= 0.0) return;
    isPosting_        = true;
    auto& pickedEntry = pickEntry(rng_, entryBox);
    auto  it{pickedEntry.requestBox.emplace_back(budget / pickedEntry.price, pickedEntry)};
    myRequest_ = &*it;
}
}  // namespace goods_demander