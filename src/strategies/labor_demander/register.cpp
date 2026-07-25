#include "strategies/labor_demander/register.hpp"

#include <tbb/concurrent_vector.h>
#include <ranges>

#include "world/message.hpp"

namespace labor_demander {
void registerMember(RegisterMemberView view, world::Workspace& workspace) {
    if (not view.isPosting()) return;
    auto& myRequest = view.myRequest();

    int employeeCnt{};
    for (const auto i : std::views::iota(0UZ, view.offerNum())) {
        auto& entry = view.offerApplicant(i);
        if (not entry.isAccept_) continue;
        ++employeeCnt;
        entry.rosterEntry_ = view.addRoster(myRequest.wage_, view.myCompanyBoard(), workspace);
    }

    view.updateLedger(myRequest.wage_, myRequest.entryBox_.size(), employeeCnt);
}

void acceptResignation(AcceptResignationView view) {
    auto& resignationBox = view.resignationBox();
    for (const SafePtr<world::RosterEntry> resignEntry : resignationBox) {
        resignEntry->isOccupied_ = false;
        view.wageMinus(resignEntry->wage_);
        view.addEmptyRosterPool(resignEntry);
    }
}
}  // namespace labor_demander