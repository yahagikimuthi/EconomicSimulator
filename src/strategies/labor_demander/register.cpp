#include "strategies/labor_demander.hpp"

#include "world/message.hpp"

namespace labor_demander {
[[nodiscard]] auto registerMember(RegisterMemberView view) -> double {
    const auto myRequest = view.getMyRequest();

    double sumProductPower{};
    int    employeeCnt{};
    for (const auto entryRef : view.getOfferApplicants()) {
        auto& entry = entryRef.get();
        if (not entry.isAccept_) continue;
        sumProductPower += entry.productPower_;
        ++employeeCnt;
    }

    view.setLog(myRequest->wage_, view.getTargetEmploy(), employeeCnt);
    return sumProductPower;
}
}  // namespace labor_demander