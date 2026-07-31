#include "components/goods_supplier.hpp"

#include <cassert>
#include <cmath>

namespace goods_supplier {
auto Producer::calcDesiredEmploy(
    const double targetSupply, const double lastSupply, const int employeeCnt
) const -> int {
    const double targetProduction{targetSupply - inventory_};
    const double avgProductPower{
        (employeeCnt != 0.0) ? lastSupply / employeeCnt : firmProductPower_
    };
    const double desiredEmploy{(avgProductPower != 0.0) ? targetProduction / avgProductPower : 1.0};
    return static_cast<int>(std::round(desiredEmploy));
}

auto GoodsSupplier::calcDesiredEmploy(const int employeeCnt) const -> int {
    return producer_.calcDesiredEmploy(planner_.targetSupply(), planner_.lastSupply(), employeeCnt);
}
}  // namespace goods_supplier