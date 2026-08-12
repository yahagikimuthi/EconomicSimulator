#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/trader.hpp"
#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace abm::consumer_goods::supplier {
class Trader final : public base_goods::supplier::Trader<Market::consumerGoods> {
  public:
    using base_goods::supplier::Trader<Market::consumerGoods>::Trader;

    void post(
        const GoodsQuantity                         supply,
        const Price                                 pricePlan,
        tbb::concurrent_vector<ConsumerGoodsEntry>& entryBox
    ) {
        if (supply == GoodsQuantity{0.0}) return;
        isPosting_        = true;
        ledger_.inventory = supply;
        auto it{entryBox.emplace_back(pricePlan, supply)};
        myEntry_ = *it;
    }
};
}  // namespace abm::consumer_goods::supplier