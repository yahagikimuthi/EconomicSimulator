#include "strategies/labor_demander.hpp"

#include "world/message.hpp"

namespace labor_demander {
[[nodiscard]] auto registerMember(RegisterMemberView view) -> double {
    if (not view.isPosting()) return 0.0;
    const auto& myRequest = view.myRequest();

    double sumProductPower{};
    int    employeeCnt{};
    for (const auto entryRef : view.offerApplicants()) {
        auto& entry = entryRef.get();
        if (not entry.isAccept_) continue;
        sumProductPower += entry.productPower_;
        ++employeeCnt;
    }

    view.updateLedger(myRequest.wage_, employeeCnt);
    return sumProductPower;
}
}  // namespace labor_demander