#include "components/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <memory>

#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander {
void Recruiter::registerMember(HasAddRoster auto& hasAddRoster, world::Workspace& workspace) {
    if (not isPosting_) return;

    int employCnt{};
    for (SafePtr<world::LaborEntry> offeredApplicant : offerApplicants_) {
        if (not offeredApplicant->isAccept) continue;
        offeredApplicant->rosterEntry =
            hasAddRoster.addRoster(offeredApplicant->hholdID, myRequest_->wage, workspace);
        ++employCnt;
    }
    ledger_.employing += employCnt;
}

template void
Recruiter::registerMember<HumanResourceManager>(HumanResourceManager&, world::Workspace&);

auto HumanResourceManager::addRoster(const int id, const double wage, world::Workspace& workspace)
    -> SafePtr<world::RosterEntry> {
    if (emptyRosterPool_.empty())
        return &companyBoard_.roster.emplace_back(id, wage, companyBoard_, workspace);
    world::RosterEntry* newRoster = emptyRosterPool_.back().get();
    std::destroy_at(newRoster);
    std::construct_at(newRoster, id, wage, companyBoard_, workspace);
    emptyRosterPool_.pop_back();
    return newRoster;
}

void HumanResourceManager::acceptResignation() {
    auto& resignationBox = companyBoard_.resignationBox;
    for (const SafePtr<world::RosterEntry> resignEntry : resignationBox) {
        resignEntry->isOccupied = false;
        emptyRosterPool_.emplace_back(resignEntry);
    }
}
}  // namespace labor_demander