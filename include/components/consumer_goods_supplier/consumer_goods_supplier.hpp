#pragma once

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>

#include "components/consumer_goods_supplier/planner.hpp"
#include "components/consumer_goods_supplier/producer.hpp"
#include "components/consumer_goods_supplier/trader.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace consumer_goods::supplier {
class [[nodiscard]] ConsumerGoodsSupplier {
  public:
    ConsumerGoodsSupplier(const Planner&& planner, const Trader&& trader, const Producer&& producer)
        : planner_{planner}, trader_{trader}, producer_{producer} {}

    void post(const Money totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox) {
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

    void endStep(world::CensusDropBox& dropBox) {
        planner_.endStep(trader_.totalDemand(), trader_.inventory(), dropBox);
        producer_.endStep(trader_.inventory(), dropBox);
        trader_.endStep();
    }

    auto sales() const -> Money { return trader_.sales(); }

    auto workspace() -> world::Workspace& { return producer_.workspace(); }

  private:
    Planner  planner_;
    Trader   trader_;
    Producer producer_;
};
}  // namespace consumer_goods::supplier