#include "strategies/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>

#include "world/message.hpp"

namespace labor_demander {
namespace {
void sortApplicants(
    const int                                               employ,
    std::vector<std::reference_wrapper<world::LaborEntry>>& applicants,
    tbb::concurrent_vector<world::LaborEntry>&              entryBox
) {
    const std::size_t k{std::min(entryBox.size(), static_cast<std::size_t>(employ))};
    applicants.clear();
    for (world::LaborEntry& entry : entryBox) applicants.emplace_back(std::ref(entry));
    const bool isOver{entryBox.size() > static_cast<std::size_t>(employ)};

    if (not isOver) return;

    std::ranges::nth_element(
        applicants,
        applicants.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const std::reference_wrapper<const world::LaborEntry> entryRef) -> double {
            return entryRef.get().productPower_;
        }
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

    static thread_local std::vector<std::reference_wrapper<world::LaborEntry>> applicants;
    sortApplicants(employ, applicants, myRequest.entryBox_);

    int offerNum{};
    for (auto requestRef : applicants) {
        world::LaborEntry& request = requestRef.get();
        if (offerNum >= employ) break;
        request.isOffer_ = true;
        view.recordOffer(request);
        ++offerNum;
    }
}
}  // namespace labor_demander