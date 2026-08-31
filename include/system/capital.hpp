#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::capital::supplier {
inline void supplyCapital(
    const AgentID id, BaseGoodsSupplier& supplier, BaseGoodsMarket& market
) noexcept {
    supplier.post(id, market);
}

inline void tradeCapital(FirmFinance& finance, BaseGoodsSupplier& supplier) noexcept {
    supplier.trade(finance);
}

inline void afterCapitalTrade(FirmFinance& finance, CapitalDemander& demander) noexcept {
    demander.afterTrade(finance);
}

}  // namespace abm::capital::supplier

namespace abm::capital::demander {
inline void purchaseCapital(
    const AgentID id, FirmFinance& finance, CapitalDemander& demander, Market& market
) noexcept {
    demander.request(id, finance, market);
}
}  // namespace abm::capital::demander