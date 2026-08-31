#pragma once

#include "components/capital_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "values/common.hpp"
#include "world/capital.hpp"

namespace abm::consumer_firm {
void supplyGoods(
    const AgentID id, ConsumerGoodsSupplier& supplier, ConsumerGoodsMarket& market
) noexcept {
    supplier.post(id, market);
}

void tradeGoods(ConsumerGoodsSupplier& supplier) noexcept { supplier.trade(); }

void purchaseCapital(const AgentID id, CapitalDemander& demander, CapitalMarket& market) noexcept {
    demander.request(id, market);
}

void afterCapitalTrade(CapitalDemander& demander) noexcept { demander.afterTrade(); }
}  // namespace abm::consumer_firm