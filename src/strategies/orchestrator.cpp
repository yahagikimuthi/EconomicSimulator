#include "strategies/orchestrator.hpp"

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"

#include "strategies/goods_demander/level1.hpp"
#include "strategies/goods_supplier/level1.hpp"
#include "strategies/labor_demander/level1.hpp"
#include "strategies/labor_supplier/level1.hpp"

namespace orchestrator {
void postLaborRequest(
    const agent_index::Component&                indexComp,
    goods_supplier::Component&                   goodsSupplier,
    labor_demander::Component&                   laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const int  id{indexComp.getId()};
    const bool isSold{goods_supplier::isSold({goodsSupplier})};
    labor_demander::postJob(id, isSold, requestBox, {laborDemander}, laborDemander);
}

void jobEntry(
    const agent_index::Component&                indexComp,
    labor_supplier::Component&                   laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const int id{indexComp.getId()};
    labor_supplier::jobEntry({laborSupplier}, id, requestBox);
}

void offer(labor_demander::Component& laborDemander) {
    labor_demander::offerApplicants({laborDemander});
}

void acceptOffer(labor_supplier::Component& laborSupplier) {
    labor_supplier::acceptOffer({laborSupplier});
}

void registerMember(
    goods_supplier::Component& goodsSupplier, labor_demander::Component& laborDemander
) {
    const double sumEmployeeProductPower{labor_demander::registerMember({laborDemander})};
    goodsSupplier.setSumEmployeeProductPower(sumEmployeeProductPower);
}

void postGoods(
    goods_supplier::Component&                 goodsSupplier,
    labor_demander::Component&                 laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double sumWage{laborDemander.getSumWage()};
    goods_supplier::postGoods({goodsSupplier}, sumWage, entryBox);
}

void purchase(
    hhold_finance::Component&                  financeComponent,
    goods_demander::Component&                 goodsDemander,
    labor_supplier::Component&                 laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double asset{financeComponent.getAsset()};
    const double wage{laborSupplier.getWage()};
    goods_demander::purchase({goodsDemander}, asset + wage, entryBox);
}

void trade(goods_supplier::Component& goodsSupplier) { goods_supplier::trade({goodsSupplier}); }
}  // namespace orchestrator