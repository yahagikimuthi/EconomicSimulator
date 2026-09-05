#pragma once

#include <algorithm>
#include <optional>

#include "components/common.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander::human_resource {
class HumanResource final {
  public:
    explicit HumanResource(const AgentID id, const Day operationDay) noexcept
        : companyBoard_{id, operationDay - Day{1}} {}

    [[nodiscard]] auto planAndRequestBudget(const HeadCount layOffsCnt) noexcept -> Budget {
        const auto layOffsPlan = std::min(employeeCnt(), layOffsCnt);
        layOffsPlan_           = layOffsPlan;
        const auto wageSum     = sumWage();
        const auto isEmploy    = not employeeCnt().isZero();
        const auto avgWage     = isEmploy ? wageSum.value() / employeeCnt().value() : 0.0;
        requestedBudget_       = static_cast<Budget>((employeeCnt() - layOffsPlan) * Wage{avgWage});
        return *requestedBudget_;
    }

    void revisePlan(const Budget budget) noexcept {
        ASSERT(requestedBudget_);
        ASSERT(budget <= requestedBudget_);
        const auto reqBudget = *requestedBudget_;
        requestedBudget_.reset();

        if (budget == reqBudget) return;
        const auto cutWage = static_cast<Budget>(sumWage()) - budget;
        if (not cutWage.isPositive()) return;
        const auto avgWage = static_cast<Money>(sumWage()) / employeeCnt();
        const auto layOffs = cutWage.value() / avgWage.value();
        layOffsPlan_       = HeadCount{layOffs};
    }

    void layOffs() noexcept {
        ASSERT(layOffsPlan_);
        const auto layOffsCnt = layOffsPlan_;
        ASSERT(layOffsCnt->isZeroOrMore());

        auto currentLayOffs = HeadCount{0.0};
        for (auto& entry : roster_.rawEntries()) {
            if (currentLayOffs >= layOffsCnt) break;
            if (not entry.isOccupied()) continue;
            entry.resign();
            ++currentLayOffs;
        }
        layOffsPlan_.reset();
    }

    void payWage(TryWithdrawFn auto&& withdrawFn) noexcept {
        for (auto& entry : roster_.rawEntries()) {
            if (not entry.isOccupied()) continue;
            entry.payWage(withdrawFn(static_cast<Budget>(entry.wage)));
        }
    }

    [[nodiscard]] auto addRoster(
        const AgentID id, const Wage wage, base_goods::Workspace& workspace
    ) noexcept -> RosterEntry& {
        ASSERT(wage.isPositive());
        return roster_.add(id, wage, companyBoard_, workspace);
    }

    [[nodiscard]] auto requestedBudget() const noexcept -> Budget {
        ASSERT(requestedBudget_);
        return *requestedBudget_;
    }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount { return roster_.employeeCnt(); }

    [[nodiscard]] auto sumWage() const noexcept -> Wage {
        const auto out = roster_.sumWage();
        ASSERT(out >= Wage{0.0});
        return out;
    }

  private:
    Roster                   roster_;
    CompanyBoard             companyBoard_;
    std::optional<HeadCount> layOffsPlan_;
    std::optional<Budget>    requestedBudget_;
};
}  // namespace abm::labor::demander::human_resource

namespace abm::labor::demander {
using HumanResource = human_resource::HumanResource;
}