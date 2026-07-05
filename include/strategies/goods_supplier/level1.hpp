#pragma once

#include "components/goods_supplier.hpp"

namespace goods_supplier {

struct IsSoldCtx {
    IsSoldCtx(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }
    [[nodiscard]] auto targetInvRatio() const -> double {
        return comp_.parameter_.targetInventoryRatio_;
    }
    [[nodiscard]] auto lastSupply() const -> double { return comp_.log_.supply_; }

  private:
    Component& comp_;
};

[[nodiscard]] auto isSold(const IsSoldCtx& ctx) -> bool;
}  // namespace goods_supplier