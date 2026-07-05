#include "strategies/orchestrator.hpp"

#include <tbb/concurrent_vector.h>

#include "strategies/goods_supplier/level1.hpp"
#include "strategies/labor_demander/level1.hpp"

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

void offer(labor_demander::Component& laborDemander) {
    labor_demander::offerApplicants({laborDemander});
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
    goods_supplier::postGoods({goodsSupplier}, sumWage, entryBox, goodsSupplier);
}
}  // namespace orchestrator