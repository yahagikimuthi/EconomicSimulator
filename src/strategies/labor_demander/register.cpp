#include <oneapi/tbb/concurrent_vector.h>
#include "strategies/labor_demander.hpp"

#include <ranges>

#include "world/message.hpp"

namespace labor_demander {
void registerMember(RegisterMemberView view, world::Workspace& workspace) {
    if (not view.isPosting()) return;
    auto& myRequest = view.myRequest();
    view.applicantNumPlus(myRequest.entryBox_.size());

    int   employeeCnt{};
    auto& myBoard = view.myCompanyBoard();
    auto& roster  = myBoard.roster_;
    for (const auto i : std::views::iota(0UZ, view.offerNum())) {
        auto& entry = view.offerApplicant(i);
        if (not entry.isAccept_) continue;
        ++employeeCnt;
        entry.rosterEntry_ = &roster.emplace_back(myRequest.wage_, myBoard, workspace);
    }

    view.updateLedger(myRequest.wage_, employeeCnt);
}

void acceptResignation(AcceptResignationView view) {
    auto& resignationBox = view.resignationBox();
    for (SafePtr<world::RosterEntry> resignEntry : resignationBox) {
        resignEntry->isOccupied_ = false;
        view.addEmptyRosterPool(resignEntry);
    }
}
}  // namespace labor_demander