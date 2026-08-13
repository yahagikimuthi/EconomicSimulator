#pragma once

#include "core/values/goods.hpp"

namespace abm::base_goods::supplier {
struct PostingInfo {
    const Price         price{0.0};
    const GoodsQuantity supply{0.0};
};

struct TradingResult {
    const GoodsQuantity supply;
    const GoodsQuantity soldAmount;
    const GoodsQuantity totalDemand;
    const Money         sales;
};
}  // namespace abm::base_goods::supplier