#include "components/goods_supplier.hpp"

#include <cassert>
#include <cmath>

namespace goods_supplier {
auto Producer::calcTargetProduction(const double demandForecast, const double targetInvRatio) const
    -> double {
    const double targetSupply{demandForecast / (1.0 - targetInvRatio)};
    return targetSupply - inventory_;
}

auto Producer::calcDesiredEmploy(const Planner& planner, const int employeeCnt) const -> int {
    const double targetProduction{
        calcTargetProduction(planner.demandForecast(), planner.targetInvRatio())
    };
    const double avgProductPower{
        (employeeCnt != 0.0) ? planner.lastSupply() / employeeCnt : firmProductPower_
    };
    const double desiredEmploy{(avgProductPower != 0.0) ? targetProduction / avgProductPower : 1.0};
    return static_cast<int>(std::round(desiredEmploy));
}

auto GoodsSupplier::calcDesiredEmploy(const int employeeCnt) const -> int {
    return producer_.calcDesiredEmploy(planner_, employeeCnt);
}
}  // namespace goods_supplier