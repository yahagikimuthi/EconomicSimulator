#pragma once

#include <cmath>

#include "config.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace consumer_goods::supplier {
class [[nodiscard]] Producer {
  public:
    Producer(
        world::Workspace&& workspace, const double firmProductPower, const GoodsQuantity inventory
    )
        : workspace_{std::move(workspace)},
          firmProductPower_{firmProductPower},
          inventory_{inventory} {}

    auto product() const -> GoodsQuantity { return workspace_.totalInput() + inventory_; }

    auto calcDesiredEmploy(
        const GoodsQuantity targetSupply,
        const GoodsQuantity lastSupply,
        const HeadCount     employeeCnt,
        const double        laborSupplierCnt = config::agent_count::hhold
    ) const -> HeadCount PRE(targetSupply >= GoodsQuantity{0.0})
                PRE(lastSupply >= GoodsQuantity{0.0}) PRE(employeeCnt >= HeadCount{0.0}) {
        const GoodsQuantity targetProduction{targetSupply - inventory_};
        const double        avgProductPower{
            (employeeCnt != HeadCount{0.0}) ? lastSupply.value() / employeeCnt.value()
                                                   : firmProductPower_
        };
        const HeadCount desiredEmploy{
            (avgProductPower != 0.0) ? targetProduction.value() / avgProductPower : 1.0
        };
        return HeadCount{std::min(laborSupplierCnt, std::ceil(desiredEmploy.value()))};
    }

    void endStep(const GoodsQuantity unsoldAmount, world::CensusDropBox& dropBox)
        PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        inventory_ = unsoldAmount;
        dropBox.inventories.emplace_back(inventory_.value());
        workspace_.resetInput();
    }

    auto workspace() -> world::Workspace& { return workspace_; }

  private:
    world::Workspace workspace_;
    const double     firmProductPower_;
    GoodsQuantity    inventory_;
};
}  // namespace consumer_goods::supplier