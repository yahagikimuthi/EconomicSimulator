#include "strategies/orchestrator.hpp"

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "strategies/goods_demander.hpp"
#include "strategies/goods_supplier.hpp"
#include "strategies/labor_demander.hpp"
#include "strategies/labor_supplier.hpp"
#include "world/message.hpp"

namespace orchestrator::labor {
void postLaborRequest(
    const agent_index::Component&                indexComp,
    goods_supplier::Component&                   goodsSupplier,
    labor_demander::Component&                   laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const int id{indexComp.id()};
    const int desiredEmploy{goods_supplier::calcDesiredEmploy(
        goods_supplier::CalcDesiredEmployView{goodsSupplier}, laborDemander.employeeCnt()
    )};
    if (desiredEmploy > 0) {
        labor_demander::postJob(
            id, desiredEmploy, requestBox, labor_demander::PostJobView{laborDemander}
        );
    } else if (desiredEmploy < 0) {
        labor_demander::layoffs(labor_demander::LayOffsView{laborDemander}, -desiredEmploy);
    }
}

void jobEntry(
    const agent_index::Component&                indexComp,
    labor_supplier::Component&                   laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const int id{indexComp.id()};
    labor_supplier::jobEntry(labor_supplier::JobEntryView{laborSupplier}, id, requestBox);
}

void offer(labor_demander::Component& laborDemander) {
    labor_demander::offerApplicants(labor_demander::OfferApplicantsView{laborDemander});
}

void acceptOffer(labor_supplier::Component& laborSupplier) {
    labor_supplier::acceptOffer(labor_supplier::AcceptOfferView{laborSupplier});
}

void registerMember(labor_demander::Component& laborDemander) {
    labor_demander::registerMember(labor_demander::RegisterMemberView{laborDemander});
}

void endStep(
    firm_finance::Component&   financeComp,
    labor_demander::Component& laborDemander,
    world::CensusDropBox&      dropBox
) {
    labor_demander::logging(dropBox, laborDemander);
    labor_demander::reset(laborDemander);
    financeComp.assetPlus(-laborDemander.sumWage());
}
void endStep(
    hhold_finance::Component&  financeComp,
    labor_supplier::Component& laborSupplier,
    world::CensusDropBox&      dropBox
) {
    financeComp.assetPlus(laborSupplier.wage());
    labor_supplier::logging(dropBox, laborSupplier);
    labor_supplier::reset(laborSupplier);
}
}  // namespace orchestrator::labor

namespace orchestrator::goods {
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
}  // namespace orchestrator::goods