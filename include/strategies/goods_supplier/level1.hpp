#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "core/forward.hpp"

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

struct PostGoodsCtx {
    PostGoodsCtx(Component& comp) : comp_{comp} {}
    void setMyEntry(const tbb::concurrent_vector<world::GoodsEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntry_ = it;
    }

  private:
    Component& comp_;
};

void postGoods(
    PostGoodsCtx                               ctx,
    const double                               totalCost,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    Component&                                 comp
);
}  // namespace goods_supplier