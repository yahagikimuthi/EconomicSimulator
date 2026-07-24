#include "strategies/orchestrator/labor.hpp"

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "strategies/goods_supplier/calc_employ.hpp"
#include "strategies/labor_demander/lay_offs.hpp"
#include "strategies/labor_demander/offer.hpp"
#include "strategies/labor_demander/posting.hpp"
#include "strategies/labor_demander/register.hpp"
#include "strategies/labor_supplier/accept.hpp"
#include "strategies/labor_supplier/entry.hpp"
#include "strategies/updates_loggings.hpp"
#include "world/message.hpp"

namespace labor {
void AdjustWorkforce(
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
    labor_supplier::updateRosterEntry(labor_supplier::UpdateRosterEntryView{laborSupplier});
    if (not laborSupplier.shouldSearchJob()) return;
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
    labor_demander::registerMember(
        labor_demander::RegisterMemberView{laborDemander}, goodsSupplier.workspace()
    );
}

void recordRosterEntry(labor_supplier::Component& laborSuppler) {
    if (not laborSuppler.isAcceptedOffer()) return;
    const SafePtr<world::RosterEntry> rosterEntry = {laborSuppler.acceptedEntry().rosterEntry_};
    laborSuppler.rosterEntry(rosterEntry);
}

void acceptResignation(labor_demander::Component& laborDemander) {
    labor_demander::acceptResignation(labor_demander::AcceptResignationView{laborDemander});
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
}  // namespace labor