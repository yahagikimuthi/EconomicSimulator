#pragma once

#include <utility>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/mediator.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
class RecruitSystem final {
  public:
    [[nodiscard]] explicit constexpr RecruitSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    void post(
        const AgentID   id,
        const HeadCount desiredEmploy,
        LaborMarket&    laborMarket,
        IMediator auto& mediator
    ) noexcept {
        ASSERT(desiredEmploy > HeadCount{0.0});
        const auto plan = planner_.plan(desiredEmploy, mediator);
        recruiter_.post(id, plan, laborMarket);
    }

    void offer() noexcept { recruiter_.offer(); }

    template <AddRosterFn F>
    void registerMember(F&& addRoster) noexcept {
        recruiter_.registerMember(std::forward<F>(addRoster));
    }

    void endStep(IMediator auto& mediator) noexcept {
        const auto result = recruiter_.publishResult();
        if (not result) return;
        mediator.publishRecruitResult(*result);
    }

    void reset() noexcept {
        planner_.reset();
        recruiter_.reset();
    }

  private:
    RecruitPlanner planner_;
    Recruiter      recruiter_;
};

class LaborDemander final {
  public:
    [[nodiscard]] explicit constexpr LaborDemander(
        RandomGenerator& masterRng, CompanyBoard&& board
    ) noexcept
        : recruitSystem_{masterRng}, humanResource_{std::move(board)} {}

    void setMediator() noexcept {
        recruitSystem_.acceptMediator(mediator_);
        mediator_.subscribeRecruitPlan(memory_);
        mediator_.subscribeEmployPlan(memory_);
    }

    void adjustWorkforce(
        const AgentID id, const HeadCount adjustment, LaborMarket& laborMarket
    ) noexcept {
        if (adjustment > HeadCount{0.0}) {
            recruitSystem_.post(id, adjustment, laborMarket, mediator_);
        } else if (adjustment < HeadCount{0.0}) {
            humanResource_.layOffs(-adjustment);
        }
    }

    void offer() noexcept { recruitSystem_.offer(); }

    void registerMember(Workspace& workspace) noexcept {
        recruitSystem_.registerMember(
            [&] [[nodiscard]] (const AgentID id, const Wage wage) -> RosterEntry& {
                return humanResource_.addRoster(id, wage, workspace);
            }
        );
    };

    void acceptResignation() noexcept { humanResource_.acceptResignation(); }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount {
        const auto out = humanResource_.employeeCnt();
        ASSERT(out >= HeadCount{0.0});
        return out;
    }

    [[nodiscard]] auto sumWage() const noexcept -> Money {
        const auto out = humanResource_.sumWage();
        ASSERT(out >= Wage{0.0});
        return static_cast<Money>(out);
    }

    void endStep(CensusDropBox& dropBox) noexcept {
        recruitSystem_.endStep(mediator_);
        memory_.logging(dropBox);
        dropBox.employments.emplace_back(employeeCnt().value());
        dropBox.employments.emplace_back(sumWage().value());
        reset();
    }

  private:
    void reset() noexcept { recruitSystem_.reset(); }

    RecruitSystem recruitSystem_;
    HumanResource humanResource_;
    Mediator      mediator_;
    CentralMemory memory_;
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemander = labor::demander::LaborDemander;
}