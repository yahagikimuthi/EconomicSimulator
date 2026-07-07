#include "strategies/labor_supplier/level1.hpp"

#include "config/contract.hpp"
#include "strategies/labor_supplier/level2.hpp"
#include "world/message.hpp"

namespace labor_supplier {
void jobEntry(
    JobEntryView view, const int id, tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    if (requestBox.empty()) {
        view.isPosting(false);
        return;
    }
    static thread_local std::vector<std::size_t> pickSampleIdxs;
    pickSample(requestBox, pickSampleIdxs);
    sortSample(requestBox, pickSampleIdxs);

    const double productPower{view.productPower()};
    view.clearEntry();
    for (std::size_t i : pickSampleIdxs) {
        auto& request  = ACCESS(requestBox, i);
        auto& entryBox = request.entryBox_;
        view.entry(request, entryBox.emplace_back(id, productPower));
    }
}

void acceptOffer(AcceptOfferView view) {
    for (std::size_t i{}; i < view.myEntryCnt(); ++i) {
        auto& [requestRef, myEntry] = view.getMyEntry(i);
        auto& request               = requestRef.get();
        if (not myEntry->isAccept_) continue;
        view.setContraction(request.firmID_, request.wage_);
        return;
    }
}
}  // namespace labor_supplier