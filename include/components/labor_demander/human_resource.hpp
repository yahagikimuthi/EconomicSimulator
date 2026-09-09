#pragma once

#include <algorithm>
#include <cassert>
#include <optional>

#include "components/common.hpp"
#include "components/labor_demander/common.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
class HumanResource final {
  public:
    explicit HumanResource(const AgentID id) noexcept : companyBoard_{id} {}

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
        assert(requestedBudget_);
        assert(budget <= requestedBudget_);
        assert(budget.isZeroOrMore());
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
        assert(layOffsPlan_);
        const auto layOffsCnt = layOffsPlan_;
        assert(layOffsCnt->isZeroOrMore());

        auto currentLayOffs = HeadCount{0.0};
        for (auto& entry : roster_.rawEntries()) {
            if (currentLayOffs >= layOffsCnt) break;
            if (not entry.isOccupied()) continue;
            entry.resign();
            ++currentLayOffs;
        }
        layOffsPlan_.reset();
    }

    void payWage(TryPayWageFn auto&& payWageFn) noexcept {
        for (auto& entry : roster_.validEntries()) {
            entry.payWage(payWageFn(entry.wage));
        }
    }

    [[nodiscard]] auto makeAddRosterFn(base_goods::Workspace& workspace) noexcept -> AddRosterFn
        auto {
        return [&] [[nodiscard]] (const AgentID id, const Wage wage) noexcept -> RosterEntry& {
            assert(wage.isPositive());
            return roster_.add(id, wage, companyBoard_, workspace);
        };
    }

    [[nodiscard]] auto requestedBudget() const noexcept -> Budget {
        assert(requestedBudget_);
        return *requestedBudget_;
    }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount { return roster_.employeeCnt(); }

    [[nodiscard]] auto sumWage() const noexcept -> Wage {
        const auto out = roster_.sumWage();
        assert(out >= Wage{0.0});
        return out;
    }

  private:
    Roster                   roster_;
    CompanyBoard             companyBoard_;
    std::optional<HeadCount> layOffsPlan_;
    std::optional<Budget>    requestedBudget_;
};
}  // namespace abm::labor::demander