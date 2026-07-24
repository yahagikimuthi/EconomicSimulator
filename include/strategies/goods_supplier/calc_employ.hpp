#pragma once

#include "components/goods_supplier.hpp"

namespace goods_supplier::internal {
struct CalcTargetProductionView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto inventory() const -> double { return comp_.production_.inventory_; }
    auto targetInvRatio() const -> double { return comp_.parameter_.targetInventoryRatio_; }
    auto demandForecast() const -> double { return comp_.log_.demandForecast_; }
};

[[nodiscard]] auto calcTargetProduction(const CalcTargetProductionView& view) -> double;
}  // namespace goods_supplier::internal

namespace goods_supplier {
struct [[nodiscard]] CalcDesiredEmployView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto firmProductivity() const -> double { return comp_.production_.firmProductPower_; }
    auto lastSupply() const -> double { return comp_.log_.supply_; }
};

[[nodiscard]] auto calcDesiredEmploy(const CalcDesiredEmployView& view, const int employeeCnt)
    -> int;
}  // namespace goods_supplier