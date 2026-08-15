#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>

#include "components/base_goods_demander.hpp"
#include "components/util.hpp"
#include "config.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] ProductionGoodsDemander final
    : public BaseGoodsDemander<Market::productionGoods> {
  public:
    ProductionGoodsDemander(
        const detail::RandomGenerator                rng,
        const base_goods::demander::BudgetCalculator budgetCalculator
    )
        : BaseGoodsDemander<Market::productionGoods>::BaseGoodsDemander(rng, budgetCalculator) {}

    void request(
        const AgentID id, const Money asset, tbb::concurrent_vector<ProductionGoodsEntry> entryBox
    ) {
        using Entry = ProductionGoodsEntry;
        if (isPass(asset, entryBox)) return;
        const Money budget{budgetCalculator_.calcBudget(asset)};
        if (budget <= Money{0.0}) return;
        isPosting_                              = true;
        const std::optional<Entry&> pickedEntry = pickEntry(id, entryBox);
        if (not pickedEntry) return;
        myRequest_ = pickedEntry->request(GoodsQuantity{budget / pickedEntry->price});
    }

    void afterTrade() {
        if (not isPosting_) return;
        if (not myRequest_) return;
        ledger_.purchasing += myRequest_->entry.price * myRequest_->tradeAmount;
        ledger_.amount += myRequest_->tradeAmount;
    }

    void endStep() {
        myRequest_.reset();
        isPosting_ = false;
        ledger_.reset();
    }

    auto purchase() const -> Money POST(money : money >= Money{0.0}) { return ledger_.purchasing; }
    auto purchaseAmount() const -> GoodsQuantity { return ledger_.amount; }

  private:
    auto isPass(const Money asset, const tbb::concurrent_vector<ProductionGoodsEntry>& entryBox)
        const -> bool {
        if (asset <= Money{0.0}) return true;
        if (entryBox.empty()) return true;
        return false;
    }

    auto pickEntry(
        const AgentID                                id,
        tbb::concurrent_vector<ProductionGoodsEntry> entryBox,
        const int sampleCnt = config::goods_demander::goodsSampleCnt
    ) const -> std::optional<ProductionGoodsEntry&> {
        using Entry = ProductionGoodsEntry;
        auto toDouble{[](const Entry& entry) -> double { return entry.supply.value(); }};
        std::optional<Entry&> betterEntry{std::nullopt};

        for (const auto _ : std::views::iota(0, sampleCnt)) {
            auto&      sampleEntry = rng_.discreteDistribution(entryBox, toDouble);
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

    struct {
        Money         purchasing{0.0};
        GoodsQuantity amount{0.0};
        void          reset() { purchasing = Money{0.0}, amount = GoodsQuantity{0.0}; }
    } ledger_;
};
}  // namespace abm