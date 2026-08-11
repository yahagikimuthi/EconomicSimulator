#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <vector>

#include "components/labor_demander/concepts.hpp"
#include "components/labor_demander/planner.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander::internal {
inline auto sortApplicants(const HeadCount offer, tbb::concurrent_vector<LaborEntry>& entryBox)
    -> auto {
    using EntryRef = std::reference_wrapper<LaborEntry>;
    static thread_local std::vector<EntryRef> applicants;
    applicants.clear();
    const std::size_t k{std::min(entryBox.size(), static_cast<std::size_t>(offer.value()))};
    for (LaborEntry& entry : entryBox) applicants.emplace_back(std::ref(entry));
    const bool isOver{entryBox.size() > static_cast<std::size_t>(offer.value())};

    const auto toRawRef{[](EntryRef entryRef) -> LaborEntry& { return entryRef.get(); }};
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
class [[nodiscard]] Recruiter {
  public:
    Recruiter(const RequestPlanner& planner) : planner_{planner} {}

    void post(
        const AgentID                         id,
        const HeadCount                       desiredEmploy,
        tbb::concurrent_vector<LaborRequest>& requestBox
    ) PRE(desiredEmploy >= HeadCount{0.0}) {
        isRecruiting_ = true;
        planner_.judgePlan(desiredEmploy);
        if (planner_.offerPlan() == HeadCount{0.0}) return;
        isPosting_             = true;
        ledger_.remainOfferNum = planner_.offerPlan();
        auto it{requestBox.emplace_back(id, planner_.wagePlan())};
        myRequest_ = *it;
    }

    void offer() {
        if (not isPosting_) return;
        if (myRequest_->entryBox.empty()) return;

        auto applicants{
            internal::sortApplicants(ledger_.remainOfferNum, myRequest_->entryBox) |
            std::views::take(ledger_.remainOfferNum.value())
        };
        for (auto&& entry : applicants) {
            entry.isOffer = true;
            offerApplicants_.emplace_back(std::ref(entry));
            --ledger_.remainOfferNum;
        }
        ledger_.applicantNum += HeadCount{static_cast<double>(myRequest_->entryBox.size())};
    }

    void registerMember(AddRosterFn auto&& addRoster) {
        using Entry = LaborEntry;
        if (not isPosting_) return;

        HeadCount              employCnt{0.0};
        std::ranges::view auto acceptApplicants{
            offerApplicants_ | std::views::filter(&Entry::isAccept)
        };
        for (LaborEntry& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry = addRoster(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledger_.employing += employCnt;
    }

    void endStep(CensusDropBox& dropBox) {
        if (not isRecruiting_) return;
        planner_.endStep(dropBox, ledger_.employing, ledger_.applicantNum);
        myRequest_.reset();
        offerApplicants_.clear();
        isPosting_ = false;
        ledger_.reset();
        isRecruiting_ = false;
    }

  private:
    template <typename U>
    using RefWrapper = std::reference_wrapper<U>;

    RequestPlanner                      planner_;
    std::optional<LaborRequest&>        myRequest_{std::nullopt};
    std::vector<RefWrapper<LaborEntry>> offerApplicants_;
    bool                                isPosting_{false};

    struct {
        HeadCount remainOfferNum{0.0};
        HeadCount applicantNum{0.0};
        HeadCount employing{0.0};

        void reset() {
            remainOfferNum = HeadCount{0.0};
            applicantNum   = HeadCount{0.0};
            employing      = HeadCount{0.0};
        }
    } ledger_{};

    bool isRecruiting_{false};
};
}  // namespace labor::demander