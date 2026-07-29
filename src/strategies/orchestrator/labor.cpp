#include "strategies/orchestrator/labor.hpp"

#include <tbb/concurrent_vector.h>
#include <core/base.hpp>

#include "components/common.hpp"
#include "strategies/goods_supplier/calc_employ.hpp"
#include "strategies/labor_supplier/accept.hpp"
#include "strategies/labor_supplier/entry.hpp"
#include "strategies/updates_loggings.hpp"
#include "world/message.hpp"

namespace labor {
void adjustWorkforce(
    const agent_index::Component&                indexComp,
    goods_supplier::Component&                   goodsSupplier,
    labor_demander::LaborDemander&               laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const int desiredEmploy{goods_supplier::calcDesiredEmploy(
        goods_supplier::CalcDesiredEmployView{goodsSupplier}, laborDemander.employeeCnt()
    )};
    if (desiredEmploy > 0) {
        laborDemander.post(indexComp.id(), desiredEmploy, requestBox);
    } else if (desiredEmploy < 0) {
        laborDemander.layOffs(-desiredEmploy);
    }
}

void jobEntry(
    const agent_index::Component&                indexComp,
    labor_supplier::Component&                   laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    labor_supplier::updateRosterEntry(labor_supplier::UpdateRosterEntryView{laborSupplier});
    if (not laborSupplier.shouldSearchJob()) return;
    const int id{indexComp.id()};
    labor_supplier::jobEntry(labor_supplier::JobEntryView{laborSupplier}, id, requestBox);
}

void offer(labor_demander::LaborDemander& laborDemander) { laborDemander.offer(); }

void acceptOffer(labor_supplier::Component& laborSupplier) {
    labor_supplier::acceptOffer(labor_supplier::AcceptOfferView{laborSupplier});
}

void registerMember(
    goods_supplier::Component& goodsSupplier, labor_demander::LaborDemander& laborDemander
) {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void recordRosterEntry(labor_supplier::Component& laborSuppler) {
    if (not laborSuppler.isAcceptedOffer()) return;
    const SafePtr<world::RosterEntry> rosterEntry{laborSuppler.acceptedEntry().rosterEntry};
    laborSuppler.rosterEntry(rosterEntry);
}

void acceptResignation(labor_demander::LaborDemander& laborDemander) {
    laborDemander.acceptResignation();
}

void endStep(
    firm_finance::Component&       financeComp,
    labor_demander::LaborDemander& laborDemander,
    world::CensusDropBox&          dropBox
) {
    laborDemander.endStep(dropBox);
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
}  // namespace labor