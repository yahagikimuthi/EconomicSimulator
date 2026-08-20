#pragma once

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "core/values/common.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods {
void product(LaborSupplier& laborSupplier, const Market phase) noexcept {
    laborSupplier.product(phase);
}

void postGoods(
    ConsumerGoodsSupplier& goodsSupplier,
    const LaborDemander&   laborDemander,
    ConsumerGoodsMarket&   market
) noexcept {
    goodsSupplier.post(laborDemander.sumWage(), market);
}

void purchase(
    const HHoldFinance&    finance,
    ConsumerGoodsDemander& goodsDemander,
    const LaborSupplier&   laborSupplier,
    ConsumerGoodsMarket&   market,
    const Step             step
) noexcept {
    goodsDemander.request(finance.asset() + laborSupplier.wage(), step, market);
}

void trade(ConsumerGoodsSupplier& goodsSupplier) noexcept { goodsSupplier.trade(); }

void afterTrade(ConsumerGoodsDemander& goodsDemander) noexcept { goodsDemander.afterTrade(); }

void endStep(ConsumerGoodsSupplier& goodsSupplier, CensusDropBox& dropBox) noexcept {
    goodsSupplier.endStep(dropBox);
}

void endStep(HHoldFinance& finance, ConsumerGoodsDemander& goodsDemander) noexcept {
    goodsDemander.endStep([&](const Money purchase) -> void { finance.assetPlus(-purchase); });
}
}  // namespace abm::consumer_goods