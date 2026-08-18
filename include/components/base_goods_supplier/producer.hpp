#pragma once

#include <cmath>
#include <utility>

#include "config.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::base_goods::supplier {
class Producer final {
  public:
    [[nodiscard]] Producer(
        Workspace&& workspace, const double firmProductPower, const GoodsQuantity inventory
    )
        : workspace_{std::move(workspace)},
          firmProductPower_{firmProductPower},
          inventory_{inventory} {}

    [[nodiscard]] auto product() const -> GoodsQuantity {
        return workspace_.totalInput() + inventory_;
    }

    [[nodiscard]] auto calcDesiredEmploy(
        const GoodsQuantity targetSupply,
        const GoodsQuantity lastSupply,
        const HeadCount     employeeCnt,
        const double        laborSupplierCnt = config::agent_count::hhold
    ) const -> HeadCount PRE(targetSupply >= GoodsQuantity{0.0})
                PRE(lastSupply >= GoodsQuantity{0.0}) PRE(employeeCnt >= HeadCount{0.0}) {
        const auto targetProduction = GoodsQuantity{targetSupply - inventory_};
        const auto avgProductPower  = (employeeCnt != HeadCount{0.0})
                                          ? lastSupply.value() / employeeCnt.value()
                                          : firmProductPower_;
        const auto desiredEmploy =
            HeadCount{(avgProductPower != 0.0) ? targetProduction.value() / avgProductPower : 1.0};
        return HeadCount{std::min(laborSupplierCnt, std::ceil(desiredEmploy.value()))};
    }

    void endStep(const GoodsQuantity unsoldAmount, CensusDropBox& dropBox)
        PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        inventory_ = unsoldAmount;
        dropBox.inventories.emplace_back(inventory_.value());
        workspace_.resetInput();
    }

    [[nodiscard]] auto workspace() -> Workspace& { return workspace_; }

  private:
    Workspace     workspace_;
    const double  firmProductPower_;
    GoodsQuantity inventory_;
};
}  // namespace abm::base_goods::supplier