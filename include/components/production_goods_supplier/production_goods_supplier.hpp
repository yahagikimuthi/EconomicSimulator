#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/base_goods_supplier/common.hpp"
#include "components/production_goods_supplier/trader.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm {
class ProductionGoodsSupplier final : public BaseGoodsSupplier {
  public:
    [[nodiscard]] ProductionGoodsSupplier(
        const base_goods::supplier::Planner&      planner,
        const production_goods::supplier::Trader& trader,
        base_goods::supplier::Producer&&          producer
    )
        : BaseGoodsSupplier(planner, std::move(producer)), trader_{trader} {}

    void post(
        const AgentID                                 id,
        const Money                                   totalCost,
        tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
    ) {
        const auto supply = producer_.product();
        trader_.post(id, planner_.judgePlan(supply, totalCost), entryBox);
    }

    void trade() { trader_.trade(); }

    void endStep(AssetPlusFn auto&& assetPlus, CensusDropBox& dropBox) {
        using namespace base_goods::supplier;
        const auto result = trader_.tradingResult();
        assetPlus(result.sales);
        planner_.endStep(result.totalDemand, result.supply - result.soldAmount, dropBox);
        producer_.endStep(result.supply - result.soldAmount, dropBox);
        trader_.endStep();
    }

  private:
    production_goods::supplier::Trader trader_;
};
}  // namespace abm