#pragma once

#include "components/goods_supplier.hpp"

namespace goods_supplier::detail {
struct CalcTargetProductionView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto inventory() const -> double { return comp_.production_.inventory_; }
    auto targetInvRatio() const -> double { return comp_.parameter_.targetInventoryRatio_; }
    auto demandForecast() const -> double { return comp_.log_.demandForecast_; }
};

[[nodiscard]] auto calcTargetProduction(const CalcTargetProductionView& view) -> double;
}  // namespace goods_supplier::detail