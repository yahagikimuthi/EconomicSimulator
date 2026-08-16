#pragma once

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
class LaborDemander {
  public:
    [[nodiscard]] LaborDemander(
        const planner::RequestPlanner& offerPlanner,
        const HumanResource&&          humanResource,
        const Memory&                  memory
    )
        : requestPlanner_{offerPlanner}, humanResource_{humanResource}, memory_{memory} {}

    void post(const AgentID id, const HeadCount desiredEmploy, LaborMarket& laborMarket)
        PRE(desiredEmploy > HeadCount{0.0}) {
        const auto plan{requestPlanner_.plan(desiredEmploy)};
        memory_.memorize(plan);
        recruiter_.post(id, plan, laborMarket);
    }

    void offer() { recruiter_.offer(); }

    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0}) {
        humanResource_.layOffs(layOffsCnt);
    }

    void registerMember(Workspace& workspace) {
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
        memory_.memorize(recruiter_.publishResult());
        recruiter_.endStep();
        memory_.endStep();
    }

  private:
    labor::demander::planner::RequestPlanner requestPlanner_;
    labor::demander::Recruiter               recruiter_;
    labor::demander::HumanResource           humanResource_;
    labor::demander::Memory                  memory_;
};

}  // namespace abm::labor::demander

namespace abm {
using LaborDemander = labor::demander::LaborDemander;
}