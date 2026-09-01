#pragma once

#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier/goods_supplier.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::goods::supplier {
inline void supply(const AgentID id, GoodsSupplier& supplier, BaseGoodsMarket& market) noexcept {
    supplier.post(id, market);
}

inline void trade(FirmFinance& finance, GoodsSupplier& supplier) noexcept {
    supplier.trade(finance);
}
}  // namespace abm::goods::supplier

namespace abm::goods::demander {
inline void purchase(
    const AgentID id, HHoldFinance& finance, GoodsDemander& demander, BaseGoodsMarket& market
) noexcept {
    demander.request(id, finance, market);
}

inline void afterTrade(HHoldFinance& finance, GoodsDemander& demander) noexcept {
    demander.afterTrade(finance);
}
}  // namespace abm::goods::demander