#include "components/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>

#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace {
auto sortApplicants(const HeadCount offer, tbb::concurrent_vector<world::LaborEntry>& entryBox)
    -> std::ranges::view auto {
    using EntryRef = std::reference_wrapper<world::LaborEntry>;
    static thread_local std::vector<EntryRef> applicants;
    applicants.clear();
    const std::size_t k{std::min(entryBox.size(), static_cast<std::size_t>(offer.value()))};
    for (world::LaborEntry& entry : entryBox) applicants.emplace_back(std::ref(entry));
    const bool isOver{entryBox.size() > static_cast<std::size_t>(offer.value())};

    const auto toRawRef{[](EntryRef entryRef) -> world::LaborEntry& { return entryRef.get(); }};
    if (not isOver) return applicants | std::views::transform(toRawRef);

    std::ranges::nth_element(
        applicants,
        applicants.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const EntryRef entryRef) -> double { return entryRef.get().productPower; }
    );
    return applicants | std::views::transform(toRawRef);
}
}  // namespace

namespace labor::demander {
void Recruiter::offer() {
    if (not isPosting_) return;
    if (myRequest_->entryBox.empty()) return;

    std::ranges::view auto applicants{sortApplicants(ledger_.remainOfferNum, myRequest_->entryBox)};

    for (auto&& entry : applicants) {
        if (ledger_.remainOfferNum <= HeadCount{0.0}) break;
        entry.isOffer = true;
        offerApplicants_.emplace_back(&entry);
        --ledger_.remainOfferNum;
    }
    ledger_.applicantNum += HeadCount{static_cast<double>(myRequest_->entryBox.size())};
}
}  // namespace labor::demander