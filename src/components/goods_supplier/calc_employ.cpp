#include "components/goods_supplier.hpp"

#include <cassert>
#include <cmath>

#include "config.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"

namespace goods::supplier {
auto Producer::calcDesiredEmploy(
    const GoodsQuantity targetSupply, const GoodsQuantity lastSupply, const HeadCount employeeCnt
) const -> HeadCount {
    const GoodsQuantity targetProduction{targetSupply - inventory_};
    const double        avgProductPower{
        (employeeCnt != HeadCount{0.0}) ? lastSupply.value() / employeeCnt.value()
                                               : firmProductPower_
    };
    const HeadCount desiredEmploy{
        (avgProductPower != 0.0) ? targetProduction.value() / avgProductPower : 1.0
    };
    return HeadCount{
        std::min(static_cast<double>(config::agent_count::hhold), std::round(desiredEmploy.value()))
    };
}

auto GoodsSupplier::calcDesiredEmploy(const HeadCount employeeCnt) const -> HeadCount {
    return producer_.calcDesiredEmploy(planner_.targetSupply(), planner_.lastSupply(), employeeCnt);
}
}  // namespace goods::supplier