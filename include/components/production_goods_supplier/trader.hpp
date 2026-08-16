#pragma once

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/trader.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/message.hpp"

namespace abm::production_goods::supplier {
class Trader final : public base_goods::supplier::Trader<Market::productionGoods> {
  public:
    [[nodiscard]] Trader(const RandomGenerator rng)
        : base_goods::supplier::Trader<Market::productionGoods>::Trader(rng) {}

    void post(
        const AgentID                                 id,
        const base_goods::supplier::TradePlan         postingInfo,
        tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
    ) {
        const auto [pricePlan, _, supply] = postingInfo;
        if (supply == GoodsQuantity{0.0}) return;
        isPosting_ = true;
        ledgerManager_.makeNewPage(supply);
        auto it{entryBox.emplace_back(id, pricePlan, supply)};
        myEntry_ = *it;
    }
};
}  // namespace abm::production_goods::supplier