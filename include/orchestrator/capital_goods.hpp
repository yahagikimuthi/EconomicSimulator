#pragma once

#include "components/capital_goods_demander.hpp"
#include "components/capital_goods_supplier/capital_goods_supplier.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "world/capital_goods.hpp"
#include "world/common.hpp"

namespace abm::capital_goods {
void product(LaborSupplier& laborSupplier, const EMarket phase) noexcept {
    laborSupplier.product(phase);
}

void postGoods(
    const AgentID&        id,
    CapitalGoodsSupplier& goodsSupplier,
    const LaborDemander&  laborDemander,
    CapitalGoodsMarket&   market
) noexcept {
    goodsSupplier.post(id, laborDemander.sumWage(), market);
}

void purchase(
    const AgentID&         id,
    const FirmFinance&     finance,
    CapitalGoodsDemander&  capitalGoodsDemander,
    ConsumerGoodsSupplier& consumerGoodsSupplier,
    CapitalGoodsMarket&    market
) noexcept {
    const auto desired = consumerGoodsSupplier.requiresCapitalGoods();
    capitalGoodsDemander.request(id, finance.asset(), desired, market);
}

void purchase(
    const AgentID&        id,
    const FirmFinance&    finance,
    CapitalGoodsDemander& capitalGoodsDemander,
    CapitalGoodsSupplier& capitalGoodsSupplier,
    CapitalGoodsMarket&   market
) noexcept {
    const auto desired = capitalGoodsSupplier.requiresCapitalGoods();
    capitalGoodsDemander.request(id, finance.asset(), desired, market);
}

void trade(CapitalGoodsSupplier& goodsSupplier) noexcept { goodsSupplier.trade(); }

void afterTrade(CapitalGoodsDemander& goodsDemander) noexcept { goodsDemander.afterTrade(); }

void endStep(
    FirmFinance&          finance,
    CapitalGoodsSupplier& capitalGoodsSupplier,
    CapitalGoodsDemander& capitalGoodsDemander,
    Government&           government,
    CensusDropBox&        dropBox
) noexcept {
    capitalGoodsSupplier.endStep(
        [&](const Money sales) noexcept -> void {
            const auto salesAfterTax = government.collectSalesTax(sales);
            finance.assetPlus(salesAfterTax);
        },
        dropBox
    );
    capitalGoodsDemander.endStep([&](const demander::TradeResult& result) noexcept -> void {
        finance.assetPlus(-result.purchased);
        capitalGoodsSupplier.addCapitalEquip(result.tradeAmount);
    });
}

void endStep(
    FirmFinance&           finance,
    ConsumerGoodsSupplier& consumerGoodsSupplier,
    CapitalGoodsDemander&  capitalGoodsDemander
) noexcept {
    capitalGoodsDemander.endStep([&](const demander::TradeResult& result) noexcept -> void {
        finance.assetPlus(-result.purchased);
        consumerGoodsSupplier.addCapitalEquip(result.tradeAmount);
    });
}

}  // namespace abm::capital_goods