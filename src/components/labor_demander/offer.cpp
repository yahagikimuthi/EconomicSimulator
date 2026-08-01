#include "components/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>

#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace {
void sortApplicants(
    const HeadCount                                         offer,
    std::vector<std::reference_wrapper<world::LaborEntry>>& applicants,
    tbb::concurrent_vector<world::LaborEntry>&              entryBox
) {
    const std::size_t k{std::min(entryBox.size(), static_cast<std::size_t>(offer.value()))};
    applicants.clear();
    for (world::LaborEntry& entry : entryBox) applicants.emplace_back(std::ref(entry));
    const bool isOver{entryBox.size() > static_cast<std::size_t>(offer.value())};

    if (not isOver) return;

    std::ranges::nth_element(
        applicants,
        applicants.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const std::reference_wrapper<const world::LaborEntry> entryRef) -> double {
            return entryRef.get().productPower;
        }
    );
}
}  // namespace

namespace labor_demander {
void Recruiter::offer() {
    if (not isPosting_) return;
    if (myRequest_->entryBox.empty()) {
        isPosting_ = false;
        return;
    }

    static thread_local std::vector<std::reference_wrapper<world::LaborEntry>> applicants;
    sortApplicants(ledger_.remainOfferNum, applicants, myRequest_->entryBox);

    for (auto entryRef : applicants) {
        if (ledger_.remainOfferNum <= HeadCount{0.0}) break;
        world::LaborEntry& entry = entryRef.get();
        entry.isOffer            = true;
        offerApplicants_.emplace_back(&entry);
        --ledger_.remainOfferNum;
    }
    ledger_.applicantNum += HeadCount{static_cast<double>(myRequest_->entryBox.size())};
}
}  // namespace labor_demander