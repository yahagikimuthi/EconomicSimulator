#pragma once

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "values/common.hpp"
#include "world/capital.hpp"

namespace abm::capital::supplier {
inline void supplyCapital(
    const AgentID id, CapitalSupplier& supplier, CapitalMarket& market
) noexcept {
    supplier.post(id, market);
}

inline void tradeCapital(CapitalSupplier& supplier) noexcept { supplier.trade(); }

inline void afterCapitalTrade(CapitalDemander& demander) noexcept { demander.afterTrade(); }

}  // namespace abm::capital::supplier

namespace abm::capital::demander {
inline void purchaseCapital(
    const AgentID id, CapitalDemander& demander, CapitalMarket& market
) noexcept {
    demander.request(id, market);
}
}  // namespace abm::capital::demander