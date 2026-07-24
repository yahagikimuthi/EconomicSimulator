#pragma once

#include "components/goods_supplier.hpp"

namespace goods_supplier {
struct [[nodiscard]] CalcDesiredEmployView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto firmProductivity() const -> double { return comp_.production_.firmProductPower_; }
    auto lastSupply() const -> double { return comp_.log_.supply_; }
};

[[nodiscard]] auto calcDesiredEmploy(const CalcDesiredEmployView& view, const int employeeCnt)
    -> int;
}  // namespace goods_supplier