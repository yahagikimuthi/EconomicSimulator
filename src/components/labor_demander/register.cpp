#include "components/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <memory>

#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander {

void LaborDemander::registerMember(world::Workspace& workspace) {
    if (not recruiter_.isPosting()) return;
    int employCnt{};
    for (SafePtr<world::LaborEntry> offeredApplicant : recruiter_.offerApplicants()) {
        if (not offeredApplicant->isAccept) continue;
        offeredApplicant->rosterEntry =
            hrManager_.addRoster(offeredApplicant->hholdID, recruiter_.myRequest().wage, workspace);
        ++employCnt;
    }
    recruiter_.addEmployingLedger(employCnt);
}

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