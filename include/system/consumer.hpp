#pragma once

#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "values/common.hpp"

namespace abm::consumer::supplier {
inline void supplyGoods(
    const AgentID id, ConsumerGoodsSupplier& supplier, ConsumerGoodsMarket& market
) noexcept {
    supplier.post(id, market);
}

inline void tradeGoods(ConsumerGoodsSupplier& supplier) noexcept { supplier.trade(); }
}  // namespace abm::consumer::supplier