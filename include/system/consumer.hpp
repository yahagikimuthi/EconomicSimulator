#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::consumer::supplier {
inline void supplyGoods(
    const AgentID id, BaseGoodsSupplier& supplier, base_goods::Market& market
) noexcept {
    supplier.post(id, market);
}

inline void tradeGoods(FirmFinance& finance, BaseGoodsSupplier& supplier) noexcept {
    supplier.trade(finance);
}
}  // namespace abm::consumer::supplier