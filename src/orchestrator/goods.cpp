#include "orchestrator/goods.hpp"

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"

namespace goods {
void product(labor_supplier::LaborSupplier laborSupplier) { laborSupplier.product(); }

void postGoods(
    goods_supplier::GoodsSupplier&             goodsSupplier,
    const labor_demander::LaborDemander&       laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    goodsSupplier.post(laborDemander.sumWage(), entryBox);
}

void purchase(
    const hhold_finance::Component&            finance,
    goods_demander::GoodsDemander&             goodsDemander,
    const labor_supplier::LaborSupplier&       laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  step
) {
    goodsDemander.request(finance.asset() + laborSupplier.wage(), step, entryBox);
}

void trade(goods_supplier::GoodsSupplier& goodsSupplier) { goodsSupplier.trade(); }

void afterTrade(goods_demander::GoodsDemander& goodsDemander) { goodsDemander.afterTrade(); }

void endStep(
    firm_finance::Component&       finance,
    goods_supplier::GoodsSupplier& goodsSupplier,
    world::CensusDropBox&          dropBox
) {
    finance.assetPlus(goodsSupplier.sales());
    goodsSupplier.endStep(dropBox);
}

void endStep(hhold_finance::Component& finance, goods_demander::GoodsDemander& goodsDemander) {
    finance.assetPlus(-goodsDemander.purchase());
    goodsDemander.endStep();
}
}  // namespace goods