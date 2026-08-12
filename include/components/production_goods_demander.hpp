#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>

#include "components/base_goods_demander.hpp"
#include "config.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace abm::production_goods::demander {
[[nodiscard]] inline auto pickEntry(
    pcg32&                                        rng,
    tbb::concurrent_vector<ProductionGoodsEntry>& entryBox,
    const int                                     sampleCnt = config::goods_demander::goodsSampleCnt
) -> ProductionGoodsEntry& {
    using Entry = ProductionGoodsEntry;
    auto toDouble{[](const Entry& entry) -> double { return entry.supply.value(); }};
    std::reference_wrapper<Entry> betterEntry =
        helper::discreteDistribution(entryBox, rng, toDouble);

    if (sampleCnt <= 1) return betterEntry.get();

    for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
        auto& sampleEntry = helper::discreteDistribution(entryBox, rng, toDouble);
        if (betterEntry.get().price <= sampleEntry.price) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace abm::production_goods::demander

namespace abm {
class [[nodiscard]] ProductionGoodsDemander final
    : public BaseGoodsDemander<Market::productionGoods> {
  public:
    ProductionGoodsDemander(const pcg32 rng, const double mpc)
        : BaseGoodsDemander<Market::productionGoods>::BaseGoodsDemander(rng, mpc) {}

    void request(
        const AgentID id, const Money asset, tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
    ) {
        using Entry = ProductionGoodsEntry;
        if (isPass(asset, entryBox)) return;
        const Money budget{calcBudget(asset)};
        if (budget <= Money{0.0}) return;
        isPosting_                              = true;
        const std::optional<Entry&> pickedEntry = pickEntry(id, entryBox);
        if (not pickedEntry) return;
        myRequest_ = pickedEntry->request(GoodsQuantity{budget / pickedEntry->price});
    }

  private:
    auto isPass(const Money asset, const tbb::concurrent_vector<ProductionGoodsEntry>& entryBox)
        const -> bool {
        if (asset <= Money{0.0}) return true;
        if (entryBox.empty()) return true;
        return false;
    }

    auto pickEntry(
        const AgentID                                 id,
        tbb::concurrent_vector<ProductionGoodsEntry>& entryBox,
        const int sampleCnt = config::goods_demander::goodsSampleCnt
    ) const -> std::optional<ProductionGoodsEntry&> {
        using Entry = ProductionGoodsEntry;
        auto toDouble{[](const Entry& entry) -> double { return entry.supply.value(); }};
        std::optional<Entry&> betterEntry{std::nullopt};

        for (const auto _ : std::views::iota(0, sampleCnt)) {
            auto&      sampleEntry = helper::discreteDistribution(entryBox, rng_, toDouble);
            const bool isAdopt{shouldAdopt(id, betterEntry, sampleEntry)};
            if (not isAdopt) continue;
            betterEntry = std::ref(sampleEntry);
        }
        return betterEntry;
    }

    auto shouldAdopt(
        const AgentID                                    id,
        const std::optional<const ProductionGoodsEntry&> existingEntry,
        const ProductionGoodsEntry&                      sampleEntry
    ) const -> bool {
        if (sampleEntry.id == id) return false;
        if (not existingEntry) return true;
        return sampleEntry.price <= existingEntry->price;
    }
};
}  // namespace abm