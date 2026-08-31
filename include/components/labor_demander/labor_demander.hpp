#pragma once

#include <algorithm>
#include <limits>
#include <utility>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/mediator.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"

namespace abm::labor::demander {
class RecruitSystem final {
  public:
    explicit RecruitSystem(RandomGenerator& masterRng) noexcept : planner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    [[nodiscard]] auto planAndRequestBudget(
        const HeadCount desiredEmploy, const Money salesPerWorker, IMediator auto& mediator
    ) noexcept -> Money {
        const auto plan = planner_.plan(desiredEmploy, salesPerWorker);
        // TODO ↓0値でも通達されるので、学習しないように修正が必要
        mediator.publishRecruitPlan(plan);
        plan_.emplace(plan);
        requestedBudget_ = plan.employ * plan.wage;
        return *requestedBudget_;
    }

    void revisePlan(const Money budget) noexcept {
        ASSERT(requestedBudget_);
        ASSERT(budget <= requestedBudget_);
        if (budget == requestedBudget_) return;
        if (plan_->employ.isZeroOrLess()) return;
        const auto wage = budget / plan_->employ;
        plan_.emplace(wage, plan_->employ, plan_->offer);
    }

    [[nodiscard]] auto requestedBudget() const noexcept -> Money {
        ASSERT(requestedBudget_);
        return *requestedBudget_;
    }

    void post(const AgentID id, Market& market) noexcept {
        ASSERT(plan_);
        recruiter_.post(id, *plan_, market);
    }

    void offer() noexcept { recruiter_.offer(); }

    template <AddRosterFn F>
    void registerMember(F&& addRoster) noexcept {
        recruiter_.registerMember(std::forward<F>(addRoster));
    }

    void endRecruiting(IMediator auto& mediator) noexcept {
        const auto result = recruiter_.publishResult();
        if (not result) return;
        mediator.publishRecruitResult(*result);
    }

    void reset() noexcept {
        planner_.reset();
        recruiter_.reset();
    }

  private:
    RecruitPlanner             planner_;
    Recruiter                  recruiter_;
    std::optional<RecruitPlan> plan_;
    std::optional<Money>       requestedBudget_;
};

class LaborDemander final {
  public:
    explicit LaborDemander(RandomGenerator& masterRng, CompanyBoard&& board) noexcept
        : recruitSystem_{masterRng}, humanResource_{std::move(board)} {}

    void setMediator() noexcept {
        recruitSystem_.acceptMediator(mediator_);
        mediator_.subscribeRecruitPlan(memory_);
        mediator_.subscribeEmployPlan(memory_);
    }

    [[nodiscard]] auto planAnnualAndRequestBudget(
        const HeadCount adjust, const Money salesForecast
    ) noexcept -> Money {
        const auto employee            = employeeCnt();
        const auto isEmploying         = not employee.isZero();
        const auto salesPerWorker      = isEmploying ? salesForecast.value() / employee.value()
                                                     : std::numeric_limits<double>::infinity();
        const auto recruitSystemBudget = recruitSystem_.planAndRequestBudget(
            std::max(adjust, HeadCount{0.0}), Money{salesPerWorker}, mediator_
        );
        const auto hrBudget =
            humanResource_.planAndRequestBudget(-std::min(adjust, HeadCount{0.0}));
        return recruitSystemBudget + hrBudget;
    }

    void reviseAnnualPlan(const Money budget) noexcept {
        const auto recruitSystemRequested = recruitSystem_.requestedBudget();
        const auto hrRequested            = humanResource_.requestedBudget();
        ASSERT(budget <= recruitSystemRequested + hrRequested);
        if (budget < hrRequested) {
            recruitSystem_.revisePlan(Money{0.0});
            humanResource_.revisePlan(budget);
            return;
        }
        recruitSystem_.revisePlan(budget - hrRequested);
        humanResource_.revisePlan(budget);
    }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount {
        const auto out = humanResource_.employeeCnt();
        ASSERT(out.isZeroOrMore());
        return out;
    }

    void postLaborRequest(const AgentID id, Market& market) noexcept {
        recruitSystem_.post(id, market);
    }

    void offer() noexcept { recruitSystem_.offer(); }

    void registerMember(base_goods::Workspace& workspace) noexcept {
        recruitSystem_.registerMember(
            [&] [[nodiscard]] (const AgentID id, const Wage wage) noexcept -> RosterEntry& {
                return humanResource_.addRoster(id, wage, workspace);
            }
        );
    };

    void endRecruiting() noexcept { recruitSystem_.endRecruiting(mediator_); }

    void layOffs() noexcept { humanResource_.layOffs(); }

    void acceptResignation() noexcept { humanResource_.acceptResignation(); }

    [[nodiscard]] auto sumWage() const noexcept -> Money {
        const auto out = humanResource_.sumWage();
        ASSERT(out.isZeroOrMore());
        return static_cast<Money>(out);
    }

  private:
    RecruitSystem recruitSystem_;
    HumanResource humanResource_;
    Mediator      mediator_;
    CentralMemory memory_;
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemander = labor::demander::LaborDemander;
}