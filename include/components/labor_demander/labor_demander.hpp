#pragma once

#include <utility>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/mediator.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
class RecruitSystem final {
    friend class LaborDemanderFactory;

  public:
    [[nodiscard]] explicit RecruitSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng} {}

    void post(
        const AgentID   id,
        const HeadCount desiredEmploy,
        LaborMarket&    laborMarket,
        IMediator auto& mediator
    ) noexcept {
        ASSERT(desiredEmploy > HeadCount{0.0});
        isRecruiting_   = true;
        const auto plan = planner_.plan(desiredEmploy, mediator);
        recruiter_.post(id, plan, laborMarket);
    }

    void offer() noexcept {
        if (not isRecruiting_) return;
        recruiter_.offer();
    }

    template <AddRosterFn F>
    void registerMember(F&& addRoster) noexcept {
        if (not isRecruiting_) return;
        recruiter_.registerMember(std::forward<F>(addRoster));
    }

    void endStep(IMediator auto& mediator) noexcept {
        if (not isRecruiting_) return;
        const auto result = recruiter_.publishResult();
        mediator.publishRecruitResult(result);
    }

    void reset() noexcept {
        if (not isRecruiting_) return;
        planner_.reset();
        recruiter_.reset();
        isRecruiting_ = false;
    }

  private:
    bool           isRecruiting_{false};
    RecruitPlanner planner_;
    Recruiter      recruiter_;
};

class LaborDemander final {
    friend class LaborDemanderFactory;

  public:
    [[nodiscard]] LaborDemander(RandomGenerator& masterRng, CompanyBoard&& board) noexcept
        : recruitSystem_{masterRng}, humanResource_{std::move(board)} {}

    void post(const AgentID id, const HeadCount desiredEmploy, LaborMarket& laborMarket) noexcept
        PRE(desiredEmploy > HeadCount{0.0}) {
        recruitSystem_.post(id, desiredEmploy, laborMarket, mediator_);
    }

    void offer() noexcept { recruitSystem_.offer(); }

    void layOffs(const HeadCount layOffsCnt) noexcept PRE(layOffsCnt > HeadCount{0.0}) {
        humanResource_.layOffs(layOffsCnt);
    }

    void registerMember(Workspace& workspace) noexcept {
        recruitSystem_.registerMember([&](const AgentID id, const Wage wage) -> RosterEntry& {
            return humanResource_.addRoster(id, wage, workspace);
        });
    };

    void acceptResignation() noexcept { humanResource_.acceptResignation(); }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        return humanResource_.employeeCnt();
    }

    [[nodiscard]] auto sumWage() const noexcept -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(humanResource_.sumWage());
    }

    void endStep() noexcept {
        recruitSystem_.endStep(mediator_);
        recruitSystem_.reset();
        recruitSystem_.reset();
    }

  private:
    RecruitSystem recruitSystem_;
    HumanResource humanResource_;
    Mediator      mediator_;
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemander = labor::demander::LaborDemander;
}