#pragma once

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>
#include <type_traits>

#include "components/base_goods_supplier/planner.hpp"
#include "components/base_goods_supplier/producer.hpp"
#include "components/base_goods_supplier/trader.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

template <Market SupplyGoodsType>
class [[nodiscard]] BaseGoodsSupplier {
    using Entry = std::conditional_t<
        SupplyGoodsType == Market::consumerGoods,
        ConsumerGoodsEntry,
        ProductionGoodsEntry>;

  public:
    BaseGoodsSupplier(
        const base_goods::supplier::Planner&&       planner,
        const base_goods::supplier::Trader<Entry>&& trader,
        const base_goods::supplier::Producer&&      producer
    )
        : planner_{planner}, trader_{trader}, producer_{producer} {}

    void post(const Money totalCost, tbb::concurrent_vector<Entry>& entryBox) {
        const GoodsQuantity supply{producer_.product()};
        planner_.judgePlan(supply, totalCost);
        trader_.post(supply, planner_.pricePlan(), entryBox);
    }

    void trade() { trader_.trade(); }

    auto calcDesiredEmploy(const HeadCount employeeCnt) const -> HeadCount
        PRE(employeeCnt >= HeadCount{0.0}) {
        return producer_.calcDesiredEmploy(
            planner_.targetSupply(), planner_.lastSupply(), employeeCnt
        );
    }

    void endStep(CensusDropBox& dropBox) {
        planner_.endStep(trader_.totalDemand(), trader_.inventory(), dropBox);
        producer_.endStep(trader_.inventory(), dropBox);
        trader_.endStep();
    }

    auto sales() const -> Money { return trader_.sales(); }

    auto workspace() -> Workspace& { return producer_.workspace(); }

  protected:
    base_goods::supplier::Planner       planner_;
    base_goods::supplier::Trader<Entry> trader_;
    base_goods::supplier::Producer      producer_;
};