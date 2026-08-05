#pragma once

#if __INCLUDE_LEVEL__ == 0
#include "components/labor_demander/recruiter.hpp"  // 解析時のみヘッダーを参照させる
#endif

namespace labor::demander::internal {
inline auto sortApplicants(
    const HeadCount offer, tbb::concurrent_vector<world::LaborEntry>& entryBox
) -> auto {
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
}  // namespace labor::demander::internal

namespace labor::demander {
template <IPlanner IPlanner>
void Recruiter<IPlanner>::post(
    const AgentID                                id,
    const HeadCount                              desiredEmploy,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    isRecruiting_ = true;
    planner_.judgePlan(desiredEmploy);
    if (planner_.offerPlan() == HeadCount{0.0}) return;
    isPosting_             = true;
    ledger_.remainOfferNum = planner_.offerPlan();
    auto it{requestBox.emplace_back(id, planner_.wagePlan())};
    myRequest_ = &*it;
}

template <IPlanner IPlanner>
void Recruiter<IPlanner>::endStep(world::CensusDropBox& dropBox) {
    if (not isRecruiting_) return;
    planner_.endStep(dropBox, ledger_.employing, ledger_.applicantNum);
    myRequest_ = nullptr;
    offerApplicants_.clear();
    isPosting_ = false;
    ledger_.reset();
    isRecruiting_ = false;
}

template <IPlanner IPlanner>
void Recruiter<IPlanner>::offer() {
    if (not isPosting_) return;
    if (myRequest_->entryBox.empty()) return;

    auto applicants{
        internal::sortApplicants(ledger_.remainOfferNum, myRequest_->entryBox) |
        std::views::take(ledger_.remainOfferNum.value())
    };
    for (auto&& entry : applicants) {
        entry.isOffer = true;
        offerApplicants_.emplace_back(&entry);
        --ledger_.remainOfferNum;
    }
    ledger_.applicantNum += HeadCount{static_cast<double>(myRequest_->entryBox.size())};
}
}  // namespace labor::demander