#include "strategies/orchestrator/goods.hpp"

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "strategies/goods_supplier/posting.hpp"
#include "strategies/goods_supplier/trade.hpp"
#include "strategies/labor_supplier/product.hpp"
#include "strategies/updates_loggings.hpp"

namespace goods {
void product(labor_supplier::Component laborSupplier) {
    if (not laborSupplier.isEmployed()) return;
    labor_supplier::product(labor_supplier::ProductView{laborSupplier});
}

void postGoods(
    goods_supplier::Component&                 goodsSupplier,
    const labor_demander::LaborDemander&       laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    goods_supplier::postGoods(
        goods_supplier::PostGoodsView{goodsSupplier}, laborDemander.sumWage(), entryBox
    );
}

void purchase(
    const hhold_finance::Component&            finance,
    goods_demander::GoodsDemander&             goodsDemander,
    const labor_supplier::Component&           laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  step
) {
    goodsDemander.request(finance.asset() + laborSupplier.wage(), step, entryBox);
}

void trade(goods_supplier::Component& goodsSupplier) {
    goods_supplier::trade(goods_supplier::TradeView{goodsSupplier});
}

void afterTrade(goods_demander::GoodsDemander& goodsDemander) { goodsDemander.afterTrade(); }

void endStep(
    firm_finance::Component&   financeComp,
    goods_supplier::Component& goodsSupplier,
    world::CensusDropBox&      dropBox
) {
    financeComp.assetPlus(goodsSupplier.sales());
    goods_supplier::logging(dropBox, goodsSupplier);
    goods_supplier::reset(goodsSupplier);
}
void endStep(hhold_finance::Component& finance, goods_demander::GoodsDemander& goodsDemander) {
    finance.assetPlus(-goodsDemander.purchase());
    goodsDemander.endStep();
}
}  // namespace goods