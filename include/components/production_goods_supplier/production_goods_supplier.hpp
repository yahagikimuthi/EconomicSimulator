#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/production_goods_supplier/trader.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] ProductionGoodsSupplier final
    : public BaseGoodsSupplier<Market::productionGoods> {
  public:
    ProductionGoodsSupplier(
        const base_goods::supplier::Planner&      planner,
        const production_goods::supplier::Trader& trader,
        base_goods::supplier::Producer&&          producer
    )
        : BaseGoodsSupplier<Market::productionGoods>(planner, std::move(producer)),
          trader_{trader} {}

    void post(
        const AgentID                                 id,
        const Money                                   totalCost,
        tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
    ) {
        const GoodsQuantity supply{producer_.product()};
        planner_.judgePlan(supply, totalCost);
        trader_.post(id, supply, planner_.pricePlan(), entryBox);
    }

    void trade() { trader_.trade(); }

    void endStep(CensusDropBox& dropBox) {
        planner_.endStep(trader_.totalDemand(), trader_.inventory(), dropBox);
        producer_.endStep(trader_.inventory(), dropBox);
        trader_.endStep();
    }

    auto sales() const -> Money { return trader_.sales(); }

  private:
    production_goods::supplier::Trader trader_;
};
}  // namespace abm