#include "strategies/labor_demander.hpp"

#include <ranges>

#include "world/message.hpp"

namespace labor_demander {
[[nodiscard]] auto registerMember(RegisterMemberView view) -> double {
    if (not view.isPosting()) return 0.0;
    auto& myRequest = view.myRequest();
    view.applicantNumPlus(myRequest.entryBox_.size());

    double sumProductPower{};
    int    employeeCnt{};
    auto&  myBoard = view.myCompanyBoard();
    auto&  roster  = myBoard.roster_;
    for (const auto i : std::views::iota(0UZ, view.offerNum())) {
        auto& entry = view.offerApplicant(i);
        if (not entry.isAccept_) continue;
        sumProductPower += entry.productPower_;
        ++employeeCnt;
        entry.rosterEntry_ = &roster.emplace_back(myRequest.wage_, myBoard);
    }

    view.updateLedger(myRequest.wage_, employeeCnt);
    return sumProductPower;
}
}  // namespace labor_demander