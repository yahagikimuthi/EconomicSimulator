#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "core/forward.hpp"

namespace goods_supplier {

struct IsSoldView {
    IsSoldView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }
    [[nodiscard]] auto targetInvRatio() const -> double {
        return comp_.parameter_.targetInventoryRatio_;
    }
    [[nodiscard]] auto lastSupply() const -> double { return comp_.log_.supply_; }

  private:
    Component& comp_;
};

[[nodiscard]] auto isSold(const IsSoldView& view) -> bool;

struct PostGoodsView {
    PostGoodsView(Component& comp) : comp_{comp} {}
    void setMyEntry(const tbb::concurrent_vector<world::GoodsEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntry_ = it;
    }

    void setPlan(const double price, const double supply, const double markup) {
        auto& plan  = comp_.plan_;
        plan.price_ = price, plan.markup_ = markup, plan.supply_ = supply;
    }

  private:
    Component& comp_;
};

void postGoods(
    PostGoodsView                              view,
    const double                               totalCost,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    Component&                                 comp
);

struct TradeView {
    TradeView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto getMyEntry() const -> tbb::concurrent_vector<world::GoodsEntry>::iterator {
        return comp_.posting_.myEntry_;
    }

    [[nodiscard]] auto getSupply() const -> double { return comp_.plan_.supply_; }

    void setInventory(double inventory) { comp_.production_.inventory_ = inventory; }

  private:
    Component& comp_;
};

void trade(TradeView view, Component& comp);
}  // namespace goods_supplier