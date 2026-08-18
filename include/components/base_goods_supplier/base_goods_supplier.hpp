#pragma once

#include "components/base_goods_supplier/planner.hpp"
#include "components/base_goods_supplier/producer.hpp"
#include "core/values/labor.hpp"
#include "world/goods.hpp"

namespace abm::base_goods::supplier {
class BaseGoodsSupplier {
  public:
    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employeeCnt) const noexcept -> HeadCount
        PRE(employeeCnt >= HeadCount{0.0}) {
        return producer_.calcDesiredEmploy(
            planner_.targetSupply(), planner_.lastSupply(), employeeCnt
        );
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producer_.workspace(); }

  protected:
    [[nodiscard]] BaseGoodsSupplier(const Planner& planner, const Producer&& producer) noexcept
        : planner_{planner}, producer_{producer} {}
    Planner  planner_;
    Producer producer_;
};
}  // namespace abm::base_goods::supplier

namespace abm {
using BaseGoodsSupplier = base_goods::supplier::BaseGoodsSupplier;
}