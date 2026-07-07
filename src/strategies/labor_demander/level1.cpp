#include "strategies/labor_demander/level1.hpp"

#include <tbb/concurrent_vector.h>

#include "config/contract.hpp"
#include "strategies/labor_demander/level2.hpp"
#include "world/message.hpp"

namespace labor_demander {
void postJob(
    const int                                    id,
    const bool                                   isSold,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view,
    Component&                                   comp
) {
    const double nextWage{calcNextWage({comp})};
    const int    nextEmploy{calcNextEmploy({comp}, isSold)};
    view.setPlan(nextWage, nextEmploy);
    view.setMyRequest(requestBox.emplace_back(id, nextWage));
}

void offerApplicants(OfferApplicantsView view) {
    const int                                    employ{view.getEmploy()};
    const auto                                   myRequest{view.getMyRequest()};
    static thread_local std::vector<std::size_t> sortApplicantIdxs;
    sortApplicants(employ, sortApplicantIdxs, myRequest->entryBox_);

    int offerNum{};
    view.clearOfferVec();
    for (const std::size_t i : sortApplicantIdxs) {
        if (offerNum >= employ) break;
        ACCESS(myRequest->entryBox_, i).isOffer_ = true;
        view.recordOffer(ACCESS(myRequest->entryBox_, i));
        ++offerNum;
    }
}

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