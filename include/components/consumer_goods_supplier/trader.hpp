#pragma once

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/trader.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods::supplier {
class Trader final : public base_goods::supplier::Trader<Market::consumerGoods> {
  public:
    [[nodiscard]] Trader(const RandomGenerator rng)
        : base_goods::supplier::Trader<Market::consumerGoods>::Trader(rng) {}

    void post(
        const base_goods::supplier::TradePlan       postingInfo,
        tbb::concurrent_vector<ConsumerGoodsEntry>& entryBox
    ) {
        if (postingInfo.supply == GoodsQuantity{0.0}) return;
        isPosting_ = true;
        ledgerManager_.makeNewPage(postingInfo.supply);
        auto it  = entryBox.emplace_back(postingInfo.price, postingInfo.supply);
        myEntry_ = *it;
    }
};
}  // namespace abm::consumer_goods::supplier