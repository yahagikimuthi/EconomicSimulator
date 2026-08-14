#pragma once

#include "components/base_goods_supplier/planner.hpp"
#include "components/base_goods_supplier/producer.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] BaseGoodsSupplier {
  public:
    auto calcDesiredEmploy(const HeadCount employeeCnt) const -> HeadCount
        PRE(employeeCnt >= HeadCount{0.0}) {
        return producer_.calcDesiredEmploy(
            planner_.targetSupply(), planner_.lastSupply(), employeeCnt
        );
    }

    auto workspace() -> Workspace& { return producer_.workspace(); }

  protected:
    BaseGoodsSupplier(
        const base_goods::supplier::Planner&   planner,
        const base_goods::supplier::Producer&& producer
    )
        : planner_{planner}, producer_{producer} {}
    base_goods::supplier::Planner  planner_;
    base_goods::supplier::Producer producer_;
};
}  // namespace abm