#pragma once

#include <cassert>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/offer_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "others/util.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::planner {
// 要求雇用数を雇用計画に変換するための意味論的クラス
class EmployPlanner final {
  public:
    explicit EmployPlanner() = delete("静的関数のみなのでインスタンス化は想定しない");
    [[nodiscard]] static auto plan(const HeadCount requiresRecruit) noexcept -> HeadCount {
        return requiresRecruit;
    }
};

class RecruitPlanner final {
  public:
    explicit RecruitPlanner(RandomGenerator& masterRng) noexcept
        : wagePlanner_{masterRng}, offerPlanner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        wagePlanner_.acceptMediator(mediator);
        offerPlanner_.acceptMediator(mediator);
    }

    [[nodiscard]] auto plan(const HeadCount requiresRecruit, const Money salesPerWorker) noexcept
        -> RecruitPlan {
        assert(requiresRecruit.isZeroOrMore());
        assert(salesPerWorker.isZeroOrMore());

        const auto wage   = wagePlanner_.plan(salesPerWorker);
        const auto employ = EmployPlanner::plan(requiresRecruit);
        const auto offer  = offerPlanner_.plan(employ);
        const auto plan   = RecruitPlan{.wage = wage, .employ = employ, .offer = offer};

        return plan;
    }

  private:
    WagePlanner  wagePlanner_;
    OfferPlanner offerPlanner_;
};
}  // namespace abm::labor::demander::planner

namespace abm::labor::demander {
using RecruitPlanner = planner::RecruitPlanner;
}