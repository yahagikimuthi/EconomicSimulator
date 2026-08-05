#pragma once

#include <tbb/concurrent_vector.h>
#include <ranges>

#include "components/labor_demander/concepts.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
template <IPlanner IPlanner>
class [[nodiscard]] Recruiter {
  public:
    Recruiter(const IPlanner& planner) : planner_{planner} {}

    void post(
        const AgentID                                id,
        const HeadCount                              desiredEmploy,
        tbb::concurrent_vector<world::LaborRequest>& requestBox
    ) PRE(desiredEmploy >= HeadCount{0.0});
    void offer();
    void registerMember(AddRosterFn auto&& addRoster) {
        using Entry = world::LaborEntry;
        if (not isPosting_) return;

        HeadCount              employCnt{0.0};
        std::ranges::view auto acceptApplicants{
            offerApplicants_ |
            std::views::transform([](SafePtr<Entry> entry) -> Entry& { return *entry; }) |
            std::views::filter(&Entry::isAccept)
        };
        for (auto&& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry = addRoster(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledger_.employing += employCnt;
    }
    void endStep(world::CensusDropBox& dropBox);

  private:
    IPlanner planner_;

    SafePtr<world::LaborRequest>            myRequest_{nullptr};
    std::vector<SafePtr<world::LaborEntry>> offerApplicants_;
    bool                                    isPosting_{false};

    struct {
        HeadCount remainOfferNum{0.0};
        HeadCount applicantNum{0.0};
        HeadCount employing{0.0};
        void      reset() {
            remainOfferNum = HeadCount{0.0}, applicantNum = HeadCount{0.0},
            employing = HeadCount{0.0};
        }
    } ledger_{};

    bool isRecruiting_{false};

    friend class RecruiterTester;
};
}  // namespace labor::demander

#include "components/labor_demander/recruiter.inl"