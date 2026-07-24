#pragma once

#include <pcg_random.hpp>

#include "components/goods_supplier.hpp"
#include "world/message.hpp"

namespace goods_supplier::internal {
[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double;

void shuffleIdx(
    tbb::concurrent_vector<world::GoodsRequest>&              requestBox,
    std::vector<std::reference_wrapper<world::GoodsRequest>>& requests,
    pcg32&                                                    rng
);

void performRationedTrade(
    const double supply, pcg32& rng, tbb::concurrent_vector<world::GoodsRequest>& requestBox
);

void performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox);
}  // namespace goods_supplier::internal

namespace goods_supplier {
struct [[nodiscard]] TradeView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myEntry() -> world::GoodsEntry& { return *comp_.posting_.myEntry_; }
    void inventoryMinus(double minus) { comp_.salesLedger.inventory_ -= minus; }
    void salesPlus(const double plus) { comp_.salesLedger.currentSales_ += plus; }
    void totalDemandPlus(const double plus) { comp_.salesLedger.totalDemand_ += plus; }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

void trade(TradeView view);
}  // namespace goods_supplier