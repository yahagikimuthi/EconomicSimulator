#include "strategies/orchestrator/goods.hpp"

#include "components/common.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "strategies/goods_demander/after_trade.hpp"
#include "strategies/goods_demander/request.hpp"
#include "strategies/goods_supplier/posting.hpp"
#include "strategies/goods_supplier/trade.hpp"
#include "strategies/updates_loggings.hpp"

namespace goods {
void postGoods(
    goods_supplier::Component&                 goodsSupplier,
    const labor_demander::Component&           laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double sumWage{laborDemander.sumWage()};
    goods_supplier::postGoods(goods_supplier::PostGoodsView{goodsSupplier}, sumWage, entryBox);
}

void purchase(
    const hhold_finance::Component&            financeComponent,
    goods_demander::Component&                 goodsDemander,
    const labor_supplier::Component&           laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  step
) {
    const double asset{financeComponent.asset()};
    const double wage{laborSupplier.wage()};
    goods_demander::purchase(
        goods_demander::PurchaseView{goodsDemander}, asset + wage, entryBox, step
    );
}

void trade(goods_supplier::Component& goodsSupplier) {
    goods_supplier::trade(goods_supplier::TradeView{goodsSupplier});
}

void afterTrade(goods_demander::Component& goodsDemander) {
    goods_demander::afterTrade(goods_demander::AfterTradeView{goodsDemander});
}

void endStep(
    firm_finance::Component&   financeComp,
    goods_supplier::Component& goodsSupplier,
    world::CensusDropBox&      dropBox
) {
    financeComp.assetPlus(goodsSupplier.sales());
    goods_supplier::logging(dropBox, goodsSupplier);
    goods_supplier::reset(goodsSupplier);
}
void endStep(hhold_finance::Component& financeComp, goods_demander::Component& goodsDemander) {
    financeComp.assetPlus(-goodsDemander.purchase());
    goods_demander::reset(goodsDemander);
}
}  // namespace goods