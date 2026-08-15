#pragma once

#include <optional>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] LaborDemander {
  public:
    LaborDemander(
        const labor::demander::RequestPlanner& offerPlanner,
        const labor::demander::HumanResource&& humanResource,
        const labor::demander::Memory&         memory
    )
        : requestPlanner_{offerPlanner}, humanResource_{humanResource}, memory_{memory} {}

    void post(const AgentID id, const HeadCount desiredEmploy, LaborMarket& laborMarket)
        PRE(desiredEmploy > HeadCount{0.0}) {
        const labor::demander::RecruitPlan plan{requestPlanner_.judgePlan(
            memory_.rememberLastRecruitPlan(), memory_.rememberLastRecruitResult(), desiredEmploy
        )};
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

    auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        return humanResource_.employeeCnt();
    }

    auto sumWage() const -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(humanResource_.sumWage());
    }

    void endStep(CensusDropBox& dropBox) {
        using namespace labor::demander;
        std::optional<RecruitResult> result{recruiter_.publishResult()};
        if (result) {
            memory_.memorize(*result);
        }
        recruiter_.endStep();
        memory_.endStep(dropBox);
    }

  private:
    labor::demander::RequestPlanner requestPlanner_;
    labor::demander::Recruiter      recruiter_;
    labor::demander::HumanResource  humanResource_;
    labor::demander::Memory         memory_;
};
}  // namespace abm