#pragma once

#include <utility>
#include "components/labor_demander/common.hpp"
#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
class RecruitSystem {
  public:
    [[nodiscard]] RecruitSystem(const RecruitPlanner& planner) : planner_{planner} {}

    void post(const AgentID id, const HeadCount desiredEmploy, LaborMarket& laborMarket) {
        ASSERT(desiredEmploy > HeadCount{0.0});
        isRecruiting_   = true;
        const auto plan = planner_.plan(desiredEmploy);
        recruiter_.post(id, plan, laborMarket);
    }

    void offer() {
        if (not isRecruiting_) return;
        recruiter_.offer();
    }

    template <AddRosterFn F>
    void registerMember(F&& addRoster) {
        if (not isRecruiting_) return;
        recruiter_.registerMember(std::forward<F>(addRoster));
    }

    void endStep() {
        if (not isRecruiting_) return;
        const auto result = recruiter_.publishResult();
        planner_.endStep(result);
        recruiter_.endStep();
        isRecruiting_ = false;
    }

  private:
    bool           isRecruiting_{false};
    RecruitPlanner planner_;
    Recruiter      recruiter_;
};

class LaborDemander {
  public:
    [[nodiscard]] LaborDemander(RecruitSystem&& recruitSystem, HumanResource&& humanResource)
        : recruitSystem_{std::move(recruitSystem)}, humanResource_{std::move(humanResource)} {}

    void post(const AgentID id, const HeadCount desiredEmploy, LaborMarket& laborMarket)
        PRE(desiredEmploy > HeadCount{0.0}) {
        recruitSystem_.post(id, desiredEmploy, laborMarket);
    }

    void offer() { recruitSystem_.offer(); }

    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0}) {
        humanResource_.layOffs(layOffsCnt);
    }

    void registerMember(Workspace& workspace) {
        recruitSystem_.registerMember([&](const AgentID id, const Wage wage) -> RosterEntry& {
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

    void endStep(CensusDropBox&) { recruitSystem_.endStep(); }

  private:
    RecruitSystem recruitSystem_;
    HumanResource humanResource_;
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemander = labor::demander::LaborDemander;
}