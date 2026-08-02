#include "components/goods_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "config.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace {
[[nodiscard]] auto pickEntry(
    pcg32&                                     rng,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  sampleCnt = config::goods_demander::goodsSampleCnt
) -> world::GoodsEntry& {
    const auto toDouble{[](const world::GoodsEntry& entry) -> double {
        return entry.supply.value();
    }};
    std::reference_wrapper<world::GoodsEntry> betterEntry =
        helper::discreteDistribution(entryBox, rng, toDouble);

    if (sampleCnt <= 1) return betterEntry.get();

    for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
        auto& sampleEntry = helper::discreteDistribution(entryBox, rng, toDouble);
        if (betterEntry.get().price <= sampleEntry.price) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace

namespace goods::demander {

auto GoodsDemander::isPass(
    const Money asset, const Step step, const tbb::concurrent_vector<world::GoodsEntry>& entryBox
) const -> bool {
    if (asset <= Money{0.0}) return true;
    if (entryBox.empty()) return true;
    const Step dayOfWeek{step % config::goods_demander::maxPurchaseFrequency};
    return dayOfWeek != myPhase_;
}

void GoodsDemander::request(
    const Money asset, const Step step, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    if (isPass(asset, step, entryBox)) return;
    const Money budget{calcBudget(asset)};
    if (budget <= Money{0.0}) return;
    isPosting_        = true;
    auto& pickedEntry = pickEntry(rng_, entryBox);
    auto  it{
        pickedEntry.requestBox.emplace_back(GoodsQuantity{budget / pickedEntry.price}, pickedEntry)
    };
    myRequest_ = &*it;
}
}  // namespace goods::demander