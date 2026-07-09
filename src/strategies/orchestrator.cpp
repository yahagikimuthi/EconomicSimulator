#include "strategies/orchestrator.hpp"

#include <tbb/concurrent_vector.h>
#include <components/goods_supplier.hpp>

#include "components/common.hpp"

#include "strategies/goods_demander.hpp"
#include "strategies/goods_supplier.hpp"
#include "strategies/labor_demander.hpp"
#include "strategies/labor_supplier.hpp"

namespace orchestrator {
void postLaborRequest(
    const agent_index::Component&                indexComp,
    const goods_supplier::Component&             goodsSupplier,
    labor_demander::Component&                   laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const int  id{indexComp.id()};
    const bool isSold{goodsSupplier.isSold()};
    labor_demander::postJob(id, isSold, requestBox, labor_demander::PostJobView{laborDemander});
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

void registerMember(
    goods_supplier::Component& goodsSupplier, labor_demander::Component& laborDemander
) {
    const double sumEmployeeProductPower{
        labor_demander::registerMember(labor_demander::RegisterMemberView{laborDemander})
    };
    goodsSupplier.setSumEmployeeProductPower(sumEmployeeProductPower);
}

void postGoods(
    goods_supplier::Component&                 goodsSupplier,
    const labor_demander::Component&           laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double sumWage{laborDemander.getSumWage()};
    goods_supplier::postGoods(goods_supplier::PostGoodsView{goodsSupplier}, sumWage, entryBox);
}

void purchase(
    const hhold_finance::Component&            financeComponent,
    goods_demander::Component&                 goodsDemander,
    const labor_supplier::Component&           laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double asset{financeComponent.asset()};
    const double wage{laborSupplier.getWage()};
    goods_demander::purchase(goods_demander::PurchaseView{goodsDemander}, asset + wage, entryBox);
}

void trade(goods_supplier::Component& goodsSupplier) {
    goods_supplier::trade(goods_supplier::TradeView{goodsSupplier});
}

void reset(
    labor_demander::Component& laborDemander,
    labor_supplier::Component& laborSupplier,
    goods_demander::Component& goodsDemander,
    goods_supplier::Component& goodsSupplier
) {
    labor_demander::reset(laborDemander);
    labor_supplier::reset(laborSupplier);
    goods_demander::reset(goodsDemander);
    goods_supplier::reset(goodsSupplier);
}
}  // namespace orchestrator