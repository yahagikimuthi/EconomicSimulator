#pragma once

#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "values/common.hpp"
#include "world/common.hpp"

namespace abm::consumer_goods {
void product(LaborSupplier& laborSupplier, const EMarket phase) noexcept {
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
    ConsumerGoodsMarket&   market,
    const Step             step
) noexcept {
    goodsDemander.request(finance.asset(), step, market);
}

void trade(ConsumerGoodsSupplier& goodsSupplier) noexcept { goodsSupplier.trade(); }

void afterTrade(ConsumerGoodsDemander& goodsDemander) noexcept { goodsDemander.afterTrade(); }

void endStep(
    FirmFinance&           finance,
    ConsumerGoodsSupplier& goodsSupplier,
    Government&            government,
    CensusDropBox&         dropBox
) noexcept {
    goodsSupplier.endStep(
        [&](const Money sales) noexcept -> void {
            const auto salesAfterTax = government.collectSalesTax(sales);
            finance.assetPlus(salesAfterTax);
        },
        dropBox
    );
}

void endStep(HHoldFinance& finance, ConsumerGoodsDemander& goodsDemander) noexcept {
    goodsDemander.endStep([&](const Money purchase) noexcept -> void {
        finance.assetPlus(-purchase);
    });
}
}  // namespace abm::consumer_goods