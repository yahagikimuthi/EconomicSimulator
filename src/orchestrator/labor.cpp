#include "orchestrator/labor.hpp"

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor {
void adjustWorkforce(
    const agent_index::Component&                index,
    const goods_supplier::GoodsSupplier&         goodsSupplier,
    labor_demander::LaborDemander&               laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const HeadCount desiredEmploy{goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt())};
    if (desiredEmploy > HeadCount{0.0}) {
        laborDemander.post(index.id(), desiredEmploy, requestBox);
    } else if (desiredEmploy < HeadCount{0.0}) {
        laborDemander.layOffs(-desiredEmploy);
    }
}

void jobEntry(
    const agent_index::Component&                index,
    labor_supplier::LaborSupplier&               laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    laborSupplier.entry(index.id(), requestBox);
}

void offer(labor_demander::LaborDemander& laborDemander) { laborDemander.offer(); }

void acceptOffer(labor_supplier::LaborSupplier& laborSupplier) { laborSupplier.accept(); }

void registerMember(
    goods_supplier::GoodsSupplier& goodsSupplier, labor_demander::LaborDemander& laborDemander
) {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void recordRosterEntry(labor_supplier::LaborSupplier& laborSuppler) {
    laborSuppler.recordRosterEntry();
}

void acceptResignation(labor_demander::LaborDemander& laborDemander) {
    laborDemander.acceptResignation();
}

void endStep(
    firm_finance::Component&       finance,
    labor_demander::LaborDemander& laborDemander,
    world::CensusDropBox&          dropBox
) {
    laborDemander.endStep(dropBox);
    finance.assetPlus(-laborDemander.sumWage());
}
void endStep(
    hhold_finance::Component&      finance,
    labor_supplier::LaborSupplier& laborSupplier,
    world::CensusDropBox&          dropBox
) {
    finance.assetPlus(laborSupplier.wage());
    laborSupplier.endStep(dropBox);
}
}  // namespace labor