#pragma once

#include <algorithm>
#include <limits>
#include <utility>

#include "components/common.hpp"
#include "components/labor_demander/common.hpp"
#include "components/labor_demander/human_resource.hpp"
#include "components/labor_demander/mediator.hpp"
#include "components/labor_demander/planner.hpp"
#include "components/labor_demander/recruiter.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"

namespace abm::labor::demander {
class RecruitSystem final {
  public:
    explicit RecruitSystem(RandomGenerator& masterRng) noexcept : planner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept { planner_.acceptMediator(mediator); }

    [[nodiscard]] auto requestBudget(
        const HeadCount desiredEmploy, const Money salesPerWorker
    ) noexcept -> Budget {
        const auto plan = planner_.plan(desiredEmploy, salesPerWorker);
        plan_.emplace(plan);
        requestedBudget_ = static_cast<Budget>(plan.employ * plan.wage);
        return *requestedBudget_;
    }

    void revisePlan(const Budget budget, IMediator auto& mediator) noexcept {
        ASSERT(requestedBudget_);
        ASSERT(budget <= *requestedBudget_ + Budget{global_setting::epsilon});
        requestedBudget_.reset();
        if (plan_->employ.isZero()) {
            mediator.publishRecruitPlan(*plan_);
            return;
        }
        const auto wage = budget.value() / plan_->employ.value();
        plan_.emplace(Wage{wage}, plan_->employ, plan_->offer);
        mediator.publishRecruitPlan(*plan_);
    }

    [[nodiscard]] auto requestedBudget() const noexcept -> Budget {
        ASSERT(requestedBudget_);
        return *requestedBudget_;
    }

    void post(const AgentID id, Market& market) noexcept {
        ASSERT(plan_);
        recruiter_.post(id, *plan_, market);
    }

    void offer() noexcept { recruiter_.offer(); }

    template <AddRosterFn F>
    void endRecruiting(F&& addRoster, IMediator auto& mediator) noexcept {
        plan_.reset();
        const auto result = recruiter_.endRecruiting(std::forward<F>(addRoster));
        mediator.publishRecruitResult(result);
    }

    [[nodiscard]] auto calcMonthlyCost() const noexcept -> Budget {
        if (not plan_) return Budget{0.0};
        const auto out = plan_->employ * plan_->wage / 6.0;
        ASSERT(out.isZeroOrMore());
        return static_cast<Budget>(out);
    }

    void reset() noexcept {
        planner_.reset();
        recruiter_.reset();
    }

  private:
    RecruitPlanner             planner_;
    Recruiter                  recruiter_;
    std::optional<RecruitPlan> plan_;
    std::optional<Budget>      requestedBudget_;
};

class LaborDemander final {
  public:
    explicit LaborDemander(
        const AgentID id, RandomGenerator& masterRng, const Day operationDay
    ) noexcept
        : recruitSystem_{masterRng}, humanResource_{id, operationDay} {
        setMediator();
    }
    LaborDemander(const LaborDemander& other) noexcept
        : recruitSystem_{other.recruitSystem_},
          humanResource_{other.humanResource_},
          memory_{other.memory_} {
        setMediator();
    }
    LaborDemander(LaborDemander&& other) noexcept          = delete;
    auto operator=(const LaborDemander&) -> LaborDemander& = delete;
    auto operator=(LaborDemander&&) -> LaborDemander&      = delete;
    ~LaborDemander() noexcept                              = default;

    [[nodiscard]] auto requestAnnualBudget(
        const HeadCount adjust, const Budget salesForecast
    ) noexcept -> Budget {
        const auto employee       = employeeCnt();
        const auto isEmploying    = not employee.isZero();
        const auto salesPerWorker = isEmploying ? salesForecast.value() / employee.value()
                                                : std::numeric_limits<double>::infinity();
        const auto recruitSystemBudget =
            recruitSystem_.requestBudget(std::max(adjust, HeadCount{0.0}), Money{salesPerWorker});
        const auto hrBudget =
            humanResource_.planAndRequestBudget(-std::min(adjust, HeadCount{0.0}));
        return recruitSystemBudget + hrBudget;
    }

    void reviseAnnualPlan(const Budget budget) noexcept {
        const auto recruitRequested = recruitSystem_.requestedBudget();
        const auto hrRequested      = humanResource_.requestedBudget();
        ASSERT(budget <= recruitRequested + hrRequested);
        const auto hrBudget      = std::min(hrRequested, budget);
        const auto recruitBudget = std::max(Budget{0.0}, budget - hrBudget);
        humanResource_.revisePlan(hrBudget);
        recruitSystem_.revisePlan(recruitBudget, mediator_);
    }

    void postRequest(const AgentID id, Market& market) noexcept { recruitSystem_.post(id, market); }

    void offer() noexcept { recruitSystem_.offer(); }

    void endRecruiting(base_goods::Workspace& workspace) noexcept {
        recruitSystem_.endRecruiting(
            [&](const AgentID id, const Wage wage) noexcept -> RosterEntry& {
                return humanResource_.addRoster(id, wage, workspace);
            },
            mediator_
        );
        recruitSystem_.reset();
    };

    void layOffs() noexcept { humanResource_.layOffs(); }

    template <TryWithdrawFn F>
    void payWage(F&& withdrawFn) noexcept {
        humanResource_.payWage(std::forward<F>(withdrawFn));
    }

    [[nodiscard]] auto calcMonthlyCost() const noexcept -> Budget {
        const auto recruitCost = recruitSystem_.calcMonthlyCost();
        return static_cast<Budget>(humanResource_.sumWage()) + recruitCost;
    }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount {
        const auto out = humanResource_.employeeCnt();
        ASSERT(out.isZeroOrMore());
        return out;
    }

  private:
    void setMediator() noexcept {
        recruitSystem_.acceptMediator(mediator_);
        mediator_.subscribeRecruitPlan(memory_);
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