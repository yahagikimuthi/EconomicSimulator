#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"

namespace goods_supplier {
struct [[nodiscard]] PostGoodsView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void myEntry(tbb::concurrent_vector<world::GoodsEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntry_ = &*it;
    }
    void plan(const double price, const double supply, const double markup) {
        auto& plan  = comp_.plan_;
        plan.price_ = price, plan.markup_ = markup, plan.supply_ = supply;
        comp_.salesLedger.inventory_ = supply;
    }
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
    auto laborInput() const -> double { return comp_.production_.workspace_->totalLaborInput; }
};

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
);
}  // namespace goods_supplier