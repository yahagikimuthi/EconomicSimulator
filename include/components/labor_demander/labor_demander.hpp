#pragma once

#include <tbb/concurrent_vector.h>

#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "components/labor_demander/util.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] LaborDemander {
  public:
    LaborDemander(
        const labor::demander::RequestPlanner&&       offerPlanner,
        const labor::demander::Recruiter&&            recruiter,
        const labor::demander::HumanResourceManager&& hrManager
    )
        : requestPlanner_{offerPlanner}, recruiter_{recruiter}, hrManager_{hrManager} {}

    void post(
        const AgentID                         id,
        const HeadCount                       desiredEmploy,
        tbb::concurrent_vector<LaborRequest>& requestBox
    ) PRE(desiredEmploy > HeadCount{0.0}) {
        isRecruiting_ = true;
        const labor::demander::PostingInfo info{
            requestPlanner_.judgePlan(desiredEmploy, recruiter_.lastApplicantNum())
        };
        recruiter_.post(id, info, requestBox);
    }

    void offer() { recruiter_.offer(); }

    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0}) {
        hrManager_.layOffs(layOffsCnt);
    }

    void registerMember(Workspace& workspace) {
        recruiter_.registerMember([&](const AgentID id, const Wage wage) -> RosterEntry& {
            return hrManager_.addRoster(id, wage, workspace);
        });
    };

    void acceptResignation() { hrManager_.acceptResignation(); }

    auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        return hrManager_.employeeCnt();
    }

    auto sumWage() const -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(hrManager_.sumWage());
    }

    void endStep(CensusDropBox& dropBox) {
        if (not isRecruiting_) return;
        requestPlanner_.endStep(dropBox, recruiter_.employing());
        recruiter_.endStep();
    }

  private:
    bool                                  isRecruiting_{false};
    labor::demander::RequestPlanner       requestPlanner_;
    labor::demander::Recruiter            recruiter_;
    labor::demander::HumanResourceManager hrManager_;
};
}  // namespace abm