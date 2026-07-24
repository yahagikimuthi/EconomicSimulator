#include "strategies/goods_supplier/calc_employ.hpp"

#include <cmath>

namespace goods_supplier::internal {
[[nodiscard]] auto calcTargetProduction(const CalcTargetProductionView& view) -> double {
    const double targetSupply{view.demandForecast()};
    const double targetBuffedSupply{targetSupply / (1.0 - view.targetInvRatio())};
    const double targetProduction{targetBuffedSupply - view.inventory()};
    return targetProduction;
}
}  // namespace goods_supplier::internal

namespace goods_supplier {
[[nodiscard]] auto calcDesiredEmploy(const CalcDesiredEmployView& view, const int employeeCnt)
    -> int {
    const double targetProduction{
        internal::calcTargetProduction(internal::CalcTargetProductionView{view})
    };
    const double avgProductPower{
        (employeeCnt != 0.0) ? view.lastSupply() / employeeCnt : view.firmProductivity()
    };
    const double desiredEmploy{(avgProductPower != 0.0) ? targetProduction / avgProductPower : 1.0};
    return static_cast<int>(std::round(desiredEmploy));
}
}  // namespace goods_supplier