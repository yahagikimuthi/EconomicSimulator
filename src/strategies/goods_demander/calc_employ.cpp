#include "strategies/goods_supplier.hpp"

#include <cmath>

#include "core/base.hpp"

namespace goods_supplier {
namespace {
struct CalcTargetProductionView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto inventory() const -> double { return comp_.production_.inventory_; }
    auto targetInvRatio() const -> double { return comp_.parameter_.targetInventoryRatio_; }
    auto demandForecast() const -> double { return comp_.log_.demandForecast_; }
};

[[nodiscard]] auto calcTargetProduction(const CalcTargetProductionView& view) -> double {
    const double targetSupply{view.demandForecast()};
    const double targetBuffedSupply{targetSupply / (1.0 - view.targetInvRatio())};
    const double targetProduction{targetBuffedSupply - view.inventory()};
    return targetProduction;
}
}  // namespace

[[nodiscard]] auto calcDesiredEmploy(CalcDesiredEmployView view) -> int {
    const double targetProduction{calcTargetProduction(CalcTargetProductionView{view})};
    const double desiredEmploy{std::round(targetProduction / view.firmProductivity())};
    return static_cast<int>(desiredEmploy);
}
}  // namespace goods_supplier