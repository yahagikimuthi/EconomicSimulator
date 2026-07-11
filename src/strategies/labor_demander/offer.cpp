#include "strategies/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <numeric>

#include "world/message.hpp"

namespace labor_demander {
namespace {
void sortApplicants(
    const int                                        employ,
    std::vector<std::size_t>&                        sortApplicantIdxs,
    const tbb::concurrent_vector<world::LaborEntry>& entryBox
) {
    const std::size_t k{std::min(entryBox.size(), static_cast<std::size_t>(employ))};
    sortApplicantIdxs.resize(entryBox.size());
    std::ranges::iota(sortApplicantIdxs, 0UZ);
    const bool isOver{entryBox.size() > static_cast<std::size_t>(employ)};
    if (not isOver) return;

    std::ranges::nth_element(
        sortApplicantIdxs,
        sortApplicantIdxs.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [&entryBox](const std::size_t idx) -> double { return entryBox[idx].productPower_; }
    );
}
}  // namespace

void offerApplicants(OfferApplicantsView view) {
    if (not view.isPosting()) return;
    auto& myRequest = view.myRequest();
    if (myRequest.entryBox_.empty()) {
        view.isPosting(false);
        return;
    }

    const int employ{view.employPlan()};

    static thread_local std::vector<std::size_t> sortApplicantIdxs;
    sortApplicants(employ, sortApplicantIdxs, myRequest.entryBox_);

    int offerNum{};
    for (const std::size_t i : sortApplicantIdxs) {
        if (offerNum >= employ) break;
        myRequest.entryBox_[i].isOffer_ = true;
        view.recordOffer(myRequest.entryBox_[i]);
        ++offerNum;
    }
}
}  // namespace labor_demander