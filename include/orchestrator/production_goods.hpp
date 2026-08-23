#pragma once

#include "components/common.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::production_goods {
void product(LaborSupplier& laborSupplier, const Market phase) noexcept {
    laborSupplier.product(phase);
}

void postGoods(
    const AgentIndex&        index,
    ProductionGoodsSupplier& goodsSupplier,
    const LaborDemander&     laborDemander,
    ProductionGoodsMarket&   market
) noexcept {
    goodsSupplier.post(index.id(), laborDemander.sumWage(), market);
}

void purchase(
    const AgentIndex&        index,
    const FirmFinance&       finance,
    ProductionGoodsDemander& productionGoodsDemander,
    ConsumerGoodsSupplier&   consumerGoodsSupplier,
    ProductionGoodsMarket&   market
) noexcept {
    const auto desired = consumerGoodsSupplier.requiresProductionGoods();
    productionGoodsDemander.request(index.id(), finance.asset(), desired, market);
}

void purchase(
    const AgentIndex&        index,
    const FirmFinance&       finance,
    ProductionGoodsDemander& productionGoodsDemander,
    ProductionGoodsSupplier& productionGoodsSupplier,
    ProductionGoodsMarket&   market
) noexcept {
    const auto desired = productionGoodsSupplier.requiresProductionGoods();
    productionGoodsDemander.request(index.id(), finance.asset(), desired, market);
}

void trade(ProductionGoodsSupplier& goodsSupplier) noexcept { goodsSupplier.trade(); }

void afterTrade(ProductionGoodsDemander& goodsDemander) noexcept { goodsDemander.afterTrade(); }

void endStep(
    FirmFinance&             finance,
    ProductionGoodsSupplier& productionGoodsSupplier,
    ProductionGoodsDemander& productionGoodsDemander
) noexcept {
    productionGoodsDemander.endStep([&](const demander::TradeResult& result) -> void {
        finance.assetPlus(-result.purchased);
        productionGoodsSupplier.addProductionEquip(result.tradeAmount);
    });
}

void endStep(
    FirmFinance&             finance,
    ConsumerGoodsSupplier&   consumerGoodsSupplier,
    ProductionGoodsDemander& productionGoodsDemander
) noexcept {
    productionGoodsDemander.endStep([&](const demander::TradeResult& result) -> void {
        finance.assetPlus(-result.purchased);
        consumerGoodsSupplier.addProductionEquip(result.tradeAmount);
    });
}
}  // namespace abm::production_goods