#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "config/init_setup.hpp"

namespace goods_supplier {
struct PostGoodsView;
[[nodiscard]] auto calcSupply(const PostGoodsView& view) -> double;

[[nodiscard]] auto calcMarkup(
    const PostGoodsView& view, const double epsilonMarkup = config::goods_supplier::epsilonMarkup
) -> double;

[[nodiscard]] auto judgePrice(
    const double markup,
    const double totalCost,
    const double epsilonPrice = config::goods_supplier::epsilonPrice
) -> double;

[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double;

[[nodiscard]] auto performRationedTrade(
    const double supply, tbb::concurrent_vector<world::GoodsRequest>& requestBox
) -> double;

[[nodiscard]] auto performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double;

struct TradeView;
void updateLog(TradeView& view, const double salesAmount);
}  // namespace goods_supplier