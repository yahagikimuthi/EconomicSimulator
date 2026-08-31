#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::goods::supplier {
inline void supply(
    const AgentID id, BaseGoodsSupplier& supplier, BaseGoodsMarket& market
) noexcept {
    supplier.post(id, market);
}

inline void trade(FirmFinance& finance, BaseGoodsSupplier& supplier) noexcept {
    supplier.trade(finance);
}
}  // namespace abm::goods::supplier

namespace abm::goods::demander {
inline void purchase(
    const AgentID id, HHoldFinance& finance, GoodsDemander& demander, BaseGoodsMarket& market
) noexcept {
    demander.request(id, finance, market);
}
}  // namespace abm::goods::demander