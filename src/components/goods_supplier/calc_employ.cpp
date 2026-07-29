#include "components/goods_supplier.hpp"

#include <cassert>
#include <cmath>

namespace goods_supplier {
auto Producer::calcTargetProduction(const double demandForecast, const double targetInvRatio) const
    -> double {
    const double targetSupply{demandForecast / (1.0 - targetInvRatio)};
    return targetSupply - inventory_;
}

auto Producer::calcDesiredEmploy(
    const double demandForecast,
    const double lastSupply,
    const double targetInvRatio,
    const int    employeeCnt
) const -> int {
    const double targetProduction{calcTargetProduction(demandForecast, targetInvRatio)};
    const double avgProductPower{
        (employeeCnt != 0.0) ? lastSupply / employeeCnt : firmProductPower_
    };
    const double desiredEmploy{(avgProductPower != 0.0) ? targetProduction / avgProductPower : 1.0};
    return static_cast<int>(std::round(desiredEmploy));
}

auto GoodsSupplier::calcDesiredEmploy(const int employeeCnt) const -> int {
    return producer_.calcDesiredEmploy(
        planner_.demandForecast(), planner_.lastSupply(), planner_.targetInvRatio(), employeeCnt
    );
}
}  // namespace goods_supplier