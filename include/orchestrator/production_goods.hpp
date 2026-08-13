#pragma once

#include "components/common.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "world/message.hpp"

namespace abm::production_goods {
void product(LaborSupplier& laborSupplier, const Market phase) { laborSupplier.product(phase); }

void postGoods(
    const AgentIndex&                             index,
    ProductionGoodsSupplier&                      goodsSupplier,
    const LaborDemander&                          laborDemander,
    tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
) {
    goodsSupplier.post(index.id(), laborDemander.sumWage(), entryBox);
}

void purchase(
    const AgentIndex                              index,
    const FirmFinance&                            finance,
    ProductionGoodsDemander&                      goodsDemander,
    const LaborDemander&                          laborSupplier,
    tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
) {
    goodsDemander.request(index.id(), finance.asset() - laborSupplier.sumWage(), entryBox);
}

void trade(ProductionGoodsSupplier& goodsSupplier) { goodsSupplier.trade(); }

void afterTrade(ProductionGoodsDemander& goodsDemander) { goodsDemander.afterTrade(); }

void endStep(FirmFinance& finance, ProductionGoodsSupplier& goodsSupplier, CensusDropBox& dropBox) {
    goodsSupplier.endStep([&](Money sales) -> void { finance.assetPlus(sales); }, dropBox);
}

void endStep(FirmFinance& finance, ProductionGoodsDemander& goodsDemander) {
    finance.assetPlus(-goodsDemander.purchase());
    goodsDemander.endStep();
}

}  // namespace abm::production_goods