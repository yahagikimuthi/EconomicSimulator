#pragma once

#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
class LaborDemander {
  public:
    [[nodiscard]] LaborDemander(
        const RecruitPlanner& offerPlanner, const HumanResource&& humanResource
    )
        : requestPlanner_{offerPlanner}, humanResource_{humanResource} {}

    void post(const AgentID id, const HeadCount desiredEmploy, LaborMarket& laborMarket)
        PRE(desiredEmploy > HeadCount{0.0}) {
        isRecruiting_   = true;
        const auto plan = requestPlanner_.plan(desiredEmploy);
        recruiter_.post(id, plan, laborMarket);
    }

    void offer() {
        if (not isRecruiting_) return;
        recruiter_.offer();
    }

    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0}) {
        humanResource_.layOffs(layOffsCnt);
    }

    void registerMember(Workspace& workspace) {
        if (not isRecruiting_) return;
        recruiter_.registerMember([&](const AgentID id, const Wage wage) -> RosterEntry& {
            return humanResource_.addRoster(id, wage, workspace);
        });
    };

    void acceptResignation() { humanResource_.acceptResignation(); }

    [[nodiscard]] auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        return humanResource_.employeeCnt();
    }

    [[nodiscard]] auto sumWage() const -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(humanResource_.sumWage());
    }

    void endStep(CensusDropBox&) {
        if (not isRecruiting_) return;
        const auto result = recruiter_.publishResult();
        requestPlanner_.endStep(result);
        recruiter_.endStep();
        isRecruiting_ = false;
    }

  private:
    bool           isRecruiting_{false};
    RecruitPlanner requestPlanner_;
    Recruiter      recruiter_;
    HumanResource  humanResource_;
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemander = labor::demander::LaborDemander;
}