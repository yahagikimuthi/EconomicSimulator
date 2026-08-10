#pragma once

#include <tbb/concurrent_vector.h>

#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
class [[nodiscard]] LaborDemander {
  public:
    LaborDemander(const Recruiter&& recruiter, const HumanResourceManager&& hrManager)
        : recruiter_{recruiter}, hrManager_{hrManager} {}
    void post(
        const AgentID                                id,
        const HeadCount                              desiredEmploy,
        tbb::concurrent_vector<world::LaborRequest>& requestBox
    ) PRE(desiredEmploy > HeadCount{0.0}) {
        recruiter_.post(id, desiredEmploy, requestBox);
    }
    void offer() { recruiter_.offer(); }
    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0}) {
        hrManager_.layOffs(layOffsCnt);
    }
    void registerMember(world::Workspace& workspace) {
        recruiter_.registerMember([&](const AgentID id, const Wage wage) -> world::RosterEntry& {
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
    void endStep(world::CensusDropBox& dropBox) {
        recruiter_.endStep(dropBox);
        hrManager_.endStep();
    }

  private:
    Recruiter            recruiter_;
    HumanResourceManager hrManager_;
};
}  // namespace labor::demander