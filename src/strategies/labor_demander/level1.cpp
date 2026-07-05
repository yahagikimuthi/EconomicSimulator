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
    PostJobCtx                                   ctx,
    Component&                                   comp
) {
    const double nextWage{calcNextWage({comp})};
    const int    nextEmploy{calcNextEmploy({comp}, isSold)};
    ctx.setPlan(nextWage, nextEmploy);
    ctx.setMyRequest(requestBox.emplace_back(id, nextWage));
}

void offerApplicants(OfferApplicantsCtx ctx) {
    const int                                    employ{ctx.getEmploy()};
    const auto                                   myRequest{ctx.getMyRequest()};
    static thread_local std::vector<std::size_t> sortApplicantIdxs;
    sortApplicants(employ, sortApplicantIdxs, myRequest->entryBox_);

    int offerNum{};
    ctx.clearOfferVec();
    for (const std::size_t i : sortApplicantIdxs) {
        if (offerNum >= employ) break;
        ACCESS(myRequest->entryBox_, i).isOffer_ = true;
        ++offerNum;
    }
}
}  // namespace labor_demander