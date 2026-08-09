#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <vector>

#include "components/labor_demander/concepts.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
template <IPlanner T>
class [[nodiscard]] Recruiter {
  public:
    Recruiter(const T& planner) : planner_{planner} {}

    void post(
        const AgentID                                id,
        const HeadCount                              desiredEmploy,
        tbb::concurrent_vector<world::LaborRequest>& requestBox
    ) PRE(desiredEmploy >= HeadCount{0.0});
    void offer();
    void registerMember(AddRosterFn auto&& addRoster);
    void endStep(world::CensusDropBox& dropBox);

  private:
    template <typename U>
    using RefWrapper = std::reference_wrapper<U>;

    T                                          planner_;
    std::optional<world::LaborRequest&>        myRequest_{std::nullopt};
    std::vector<RefWrapper<world::LaborEntry>> offerApplicants_;
    bool                                       isPosting_{false};

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

    template <IPlanner U>
    friend class RecruiterTester;
};
}  // namespace labor::demander

#include "components/labor_demander/recruiter.inl"