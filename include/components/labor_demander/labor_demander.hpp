#pragma once

#include <limits>
#include <utility>

#include "components/common.hpp"
#include "components/labor_demander/common.hpp"
#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/mediator.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"

namespace abm::labor::demander {
class RecruitSystem final {
  public:
    [[nodiscard]] explicit constexpr RecruitSystem(RandomGenerator& masterRng) noexcept
        : planner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    [[nodiscard]] auto planAndRequestBudget(
        const HeadCount desiredEmploy, const Money salesPerWorker, IMediator auto& mediator
    ) noexcept -> Money {
        ASSERT(desiredEmploy.isZeroOrMore());
        const auto plan = planner_.plan(desiredEmploy, salesPerWorker);
        mediator.publishRecruitPlan(plan);
        plan_.emplace(plan);
        claimedBudget_ = plan.employ * plan.wage;
        return *claimedBudget_;
    }

    void revisePlan(const Money budget) noexcept {
        const auto cut     = *claimedBudget_ - budget;
        const auto cutWage = cut / plan_->employ;
        plan_.emplace(plan_->wage - cutWage, plan_->employ, plan_->offer);
    }

    void post(const AgentID id, Market& market) noexcept { recruiter_.post(id, *plan_, market); }

    void offer() noexcept { recruiter_.offer(); }

    template <AddRosterFn F>
    void registerMember(F&& addRoster) noexcept {
        recruiter_.registerMember(std::forward<F>(addRoster));
    }

    void endingRecruiting(IMediator auto& mediator) noexcept {
        const auto result = recruiter_.publishResult();
        if (not result) return;
        mediator.publishRecruitResult(*result);
    }

    void reset() noexcept {
        planner_.reset();
        recruiter_.reset();
    }

    [[nodiscard]] auto claimedBudget() const noexcept -> Money {
        ASSERT(claimedBudget_);
        return *claimedBudget_;
    }

  private:
    RecruitPlanner             planner_;
    Recruiter                  recruiter_;
    std::optional<RecruitPlan> plan_;
    std::optional<Money>       claimedBudget_;
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

    [[nodiscard]] auto planAndRequestBudget(
        const HeadCount adjustment, const Money salesForecast
    ) noexcept -> Money {
        const auto employee       = employeeCnt();
        const auto isEmploying    = not employee.isZero();
        const auto salesPerWorker = isEmploying ? salesForecast.value() / employee.value()
                                                : std::numeric_limits<double>::infinity();
        if (adjustment.isPositive()) {
            const auto recruitSystemBudget =
                recruitSystem_.planAndRequestBudget(adjustment, Money{salesPerWorker}, mediator_);
            const auto hrBudget = humanResource_.planAndRequestBudget(HeadCount{0.0});
            return recruitSystemBudget + hrBudget;
        }
        const auto recruitSystemBudget =
            recruitSystem_.planAndRequestBudget(HeadCount{0.0}, Money{salesPerWorker}, mediator_);
        const auto hrBudget = humanResource_.planAndRequestBudget(-adjustment);
        return recruitSystemBudget + hrBudget;
    }

    void revisePlan(const Money budget) noexcept {
        const auto recruitSystemClaimed = recruitSystem_.claimedBudget();
        const auto hrClaimed            = humanResource_.claimedBudget();
        ASSERT(budget <= recruitSystemClaimed + hrClaimed);
        if (budget < hrClaimed) {
            recruitSystem_.revisePlan(Money{0.0});
            humanResource_.revisePlan(budget);
            return;
        }
        recruitSystem_.revisePlan(budget - hrClaimed);
        humanResource_.revisePlan(hrClaimed);
    }

    void adjustWorkforce(const AgentID id, Market& market) noexcept {
        recruitSystem_.post(id, market);
        humanResource_.layOffs();
    }

    void offer() noexcept { recruitSystem_.offer(); }

    void registerMember(base_goods::Workspace& workspace) noexcept {
        recruitSystem_.registerMember(
            [&] [[nodiscard]] (const AgentID id, const Wage wage) noexcept -> RosterEntry& {
                return humanResource_.addRoster(id, wage, workspace);
            }
        );
    };

    void acceptResignation() noexcept { humanResource_.acceptResignation(); }

    template <AssetMinusFn F>
    void endStep(F&& assetMinus, CensusDropBox& dropBox) noexcept {
        recruitSystem_.endingRecruiting(mediator_);
        const auto totalCost = sumWage();
        std::forward<F>(assetMinus)(totalCost);
        dropBox.sumWages.emplace_back(totalCost.value());
        reset(dropBox);
    }

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

  private:
    void reset(CensusDropBox& dropBox) noexcept {
        memory_.logging(dropBox);
        dropBox.employments.emplace_back(employeeCnt().value());
        recruitSystem_.reset();
    }

    RecruitSystem recruitSystem_;
    HumanResource humanResource_;
    Mediator      mediator_;
    CentralMemory memory_;
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemander = labor::demander::LaborDemander;
}