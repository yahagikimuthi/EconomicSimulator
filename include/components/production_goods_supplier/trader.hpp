#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/trader.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace abm::production_goods::supplier {
class Trader final : public base_goods::supplier::Trader<Market::productionGoods> {
  public:
    using base_goods::supplier::Trader<Market::productionGoods>::Trader;

    void post(
        const AgentID                                 id,
        const GoodsQuantity                           supply,
        const Price                                   pricePlan,
        tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
    ) {
        if (supply == GoodsQuantity{0.0}) return;
        isPosting_        = true;
        ledger_.inventory = supply;
        auto it{entryBox.emplace_back(id, pricePlan, supply)};
        myEntry_ = *it;
    }
};
}  // namespace abm::production_goods::supplier