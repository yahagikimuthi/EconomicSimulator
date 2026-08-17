#pragma once

#include <optional>
#include <pcg_random.hpp>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {

struct WageJudgeContext {
    const Wage      lastWage;
    const HeadCount lastEmployPlan;
    const HeadCount lastApplicants;
};

class WagePlanner {
  public:
    [[nodiscard]] WagePlanner(RandomGenerator& rng)
        : rng_{pcg32{rng.makeUint64(), rng.makeUint64()}}, adjustVol_{rng.rand(0.1, 0.2)} {}

    [[nodiscard]] auto plan(const WageJudgeContext& ctx) const -> Wage {
        const auto alpha = rng_.randNormal(0.0, adjustVol_, -1.0, 1.0);
        const auto next  = Wage{
            ctx.lastWage *
            (shouldRaise(ctx.lastEmployPlan, ctx.lastApplicants) ? 1.0 + alpha : 1.0 - alpha)
        };
        return wageGuard(next);
    }

  private:
    [[nodiscard]] auto shouldRaise(const HeadCount lastEmployPlan, const HeadCount lastApplicants)
        const -> bool {
        return lastApplicants < lastEmployPlan;
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) -> Wage {
        return Wage{std::max(wage.value(), std::numeric_limits<double>::epsilon())};
    }

    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

class WagePlannerMemory {
    template <typename T>
    class Memory {
      public:
        Memory(const T& last) : last_{last} {}
        void memorize(const T& plan) { plan_ = plan; }
        void endStep() {
            if (plan_) last_ = *plan_;
        }
        [[nodiscard]] auto rememberLog() const -> const T& { return last_; }
        [[nodiscard]] auto rememberPlan() const -> const T& {
            ASSERT(plan_);
            return *plan_;
        }

      private:
        T                last_;
        std::optional<T> plan_{std::nullopt};
    };

  public:
    [[nodiscard]] WagePlannerMemory(RandomGenerator& masterRng)
        : employPlanMemory_{HeadCount{masterRng.rand(10, 20)}},
          applicantsMemory_{HeadCount{masterRng.rand(10, 20)}},
          wagePlanMemory_{Wage{masterRng.rand(10.0, 20.0)}} {}

    void listenEmployPlan(const HeadCount employPlan) { employPlanMemory_.memorize(employPlan); }

    void listenRecruitResult(const RecruitResult& result) {
        applicantsMemory_.memorize(result.applicants);
    }

    void memorizeWagePlan(const Wage wagePlan) { wagePlanMemory_.memorize(wagePlan); }

    void endStep() {
        employPlanMemory_.endStep();
        applicantsMemory_.endStep();
    }

    [[nodiscard]] auto makeJudgeContext() const -> WageJudgeContext {
        return {
            .lastWage       = wagePlanMemory_.rememberLog(),
            .lastEmployPlan = applicantsMemory_.rememberLog(),
            .lastApplicants = applicantsMemory_.rememberLog()
        };
    }

  private:
    struct EmployPlanMemory : public Memory<HeadCount> {
    } employPlanMemory_;
    struct LastApplicants : public Memory<HeadCount> {
    } applicantsMemory_;
    struct WagePlanMemory : public Memory<Wage> {
    } wagePlanMemory_;
};

class WagePlanningSystem {
  public:
    WagePlanningSystem(RandomGenerator& masterRng) : planner_{masterRng}, memory_{masterRng} {}

    [[nodiscard]] auto plan() -> Wage {
        const auto plan = planner_.plan(memory_.makeJudgeContext());
        memory_.memorizeWagePlan(plan);
        return plan;
    }

    void endStep() { memory_.endStep(); }

  private:
    WagePlanner       planner_;
    WagePlannerMemory memory_;
};
}  // namespace abm::labor::demander::planner