#pragma once

#include "components/base_goods_supplier/planner.hpp"
#include "components/base_goods_supplier/producer.hpp"

namespace abm {
template <Market SupplyGoodsType>
class [[nodiscard]] BaseGoodsSupplier {
  public:
    BaseGoodsSupplier(
        const base_goods::supplier::Planner&   planner,
        const base_goods::supplier::Producer&& producer
    )
        : planner_{planner}, producer_{producer} {}

    auto calcDesiredEmploy(const HeadCount employeeCnt) const -> HeadCount
        PRE(employeeCnt >= HeadCount{0.0}) {
        return producer_.calcDesiredEmploy(
            planner_.targetSupply(), planner_.lastSupply(), employeeCnt
        );
    }

    auto workspace() -> Workspace& { return producer_.workspace(); }

  protected:
    base_goods::supplier::Planner  planner_;
    base_goods::supplier::Producer producer_;
};
}  // namespace abm