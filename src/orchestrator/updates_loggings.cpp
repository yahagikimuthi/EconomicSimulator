#include "orchestrator/updates_loggings.hpp"

#include <pcg_random.hpp>

#include "components/common.hpp"
#include "components/goods_supplier.hpp"
#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace firm_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) { comp.endStep(dropBox); }
}  // namespace firm_finance

namespace hhold_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) { comp.endStep(dropBox); }
}  // namespace hhold_finance

namespace goods::supplier {
auto Planner::updateDemandForecast(const GoodsQuantity totalDemand) const -> GoodsQuantity {
    return log_.demandForecast +
           (param_.demandForecastAdjustVol * (totalDemand - log_.demandForecast));
}

auto Planner::isSold(const GoodsQuantity unsoldAmount) const -> bool {
    return (plan_.supply != GoodsQuantity{0.0})
               ? unsoldAmount / plan_.supply < param_.targetInvRatio
               : true;
}

void Planner::endStep(
    const GoodsQuantity totalDemand, const GoodsQuantity unsoldAmount, world::CensusDropBox& dropBox
) {
    dropBox.prices.emplace_back(plan_.price.value());
    dropBox.supplies.emplace_back(plan_.supply.value());
    dropBox.markups.emplace_back(plan_.markup);
    log_ = {
        .markup         = plan_.markup,
        .supply         = plan_.supply,
        .demandForecast = updateDemandForecast(totalDemand),
        .isSold         = isSold(unsoldAmount)
    };
    plan_.reset();
}  // namespace goods_supplier

void GoodsSupplier::endStep(world::CensusDropBox& dropBox) {
    planner_.endStep(trader_.totalDemand(), trader_.inventory(), dropBox);
    producer_.endStep(trader_.inventory(), dropBox);
    trader_.endStep();
}
}  // namespace goods::supplier