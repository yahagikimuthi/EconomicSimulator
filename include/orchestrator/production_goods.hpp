#pragma once

#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "world/common.hpp"
#include "world/production_goods.hpp"

namespace abm::production_goods {
void product(LaborSupplier& laborSupplier, const EMarket phase) noexcept {
    laborSupplier.product(phase);
}

void postGoods(
    const AgentID&           id,
    ProductionGoodsSupplier& goodsSupplier,
    const LaborDemander&     laborDemander,
    ProductionGoodsMarket&   market
) noexcept {
    goodsSupplier.post(id, laborDemander.sumWage(), market);
}

void purchase(
    const AgentID&           id,
    const FirmFinance&       finance,
    ProductionGoodsDemander& productionGoodsDemander,
    ConsumerGoodsSupplier&   consumerGoodsSupplier,
    ProductionGoodsMarket&   market
) noexcept {
    const auto desired = consumerGoodsSupplier.requiresProductionGoods();
    productionGoodsDemander.request(id, finance.asset(), desired, market);
}

void purchase(
    const AgentID&           id,
    const FirmFinance&       finance,
    ProductionGoodsDemander& productionGoodsDemander,
    ProductionGoodsSupplier& productionGoodsSupplier,
    ProductionGoodsMarket&   market
) noexcept {
    const auto desired = productionGoodsSupplier.requiresProductionGoods();
    productionGoodsDemander.request(id, finance.asset(), desired, market);
}

void trade(ProductionGoodsSupplier& goodsSupplier) noexcept { goodsSupplier.trade(); }

void afterTrade(ProductionGoodsDemander& goodsDemander) noexcept { goodsDemander.afterTrade(); }

void endStep(
    FirmFinance&             finance,
    ProductionGoodsSupplier& productionGoodsSupplier,
    ProductionGoodsDemander& productionGoodsDemander,
    Government&              government,
    CensusDropBox&           dropBox
) noexcept {
    productionGoodsSupplier.endStep(
        [&](const Money sales) noexcept -> void {
            const auto salesAfterTax = government.collectSalesTax(sales);
            finance.assetPlus(salesAfterTax);
        },
        dropBox
    );
    productionGoodsDemander.endStep([&](const demander::TradeResult& result) noexcept -> void {
        finance.assetPlus(-result.purchased);
        productionGoodsSupplier.addProductionEquip(result.tradeAmount);
    });
}

void endStep(
    FirmFinance&             finance,
    ConsumerGoodsSupplier&   consumerGoodsSupplier,
    ProductionGoodsDemander& productionGoodsDemander
) noexcept {
    productionGoodsDemander.endStep([&](const demander::TradeResult& result) noexcept -> void {
        finance.assetPlus(-result.purchased);
        consumerGoodsSupplier.addProductionEquip(result.tradeAmount);
    });
}

}  // namespace abm::production_goods