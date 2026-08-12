#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/base_goods_supplier/planner.hpp"
#include "components/base_goods_supplier/producer.hpp"
#include "components/consumer_goods_supplier/trader.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] ConsumerGoodsSupplier final : public BaseGoodsSupplier<Market::consumerGoods> {
  public:
    ConsumerGoodsSupplier(
        const base_goods::supplier::Planner&    planner,
        const consumer_goods::supplier::Trader& trader,
        base_goods::supplier::Producer&&        producer
    )
        : BaseGoodsSupplier<Market::consumerGoods>(planner, std::move(producer)), trader_{trader} {}

    void post(const Money totalCost, tbb::concurrent_vector<ConsumerGoodsEntry>& entryBox) {
        const GoodsQuantity supply{producer_.product()};
        planner_.judgePlan(supply, totalCost);
        trader_.post(supply, planner_.pricePlan(), entryBox);
    }

    void trade() { trader_.trade(); }

    void endStep(CensusDropBox& dropBox) {
        planner_.endStep(trader_.totalDemand(), trader_.inventory(), dropBox);
        producer_.endStep(trader_.inventory(), dropBox);
        trader_.endStep();
    }

    auto sales() const -> Money { return trader_.sales(); }

  private:
    consumer_goods::supplier::Trader trader_;
};
}  // namespace abm