#pragma once

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::capital::supplier {
inline void supply(const AgentID id, CapitalSupplier& supplier, BaseGoodsMarket& market) noexcept {
    supplier.post(id, market);
}

inline void trade(FirmFinance& finance, CapitalSupplier& supplier) noexcept {
    supplier.trade(finance);
}

inline void afterTrade(FirmFinance& finance, CapitalDemander& demander) noexcept {
    demander.afterTrade(finance);
}

}  // namespace abm::capital::supplier

namespace abm::capital::demander {
inline void purchase(
    const AgentID id, FirmFinance& finance, CapitalDemander& demander, Market& market
) noexcept {
    demander.request(id, finance, market);
}

inline void afterTarde(FirmFinance& finance, CapitalDemander& demander) noexcept {
    demander.afterTrade(finance);
}
}  // namespace abm::capital::demander