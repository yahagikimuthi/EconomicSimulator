#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander::human_resource {
class HumanResource final {
  public:
    explicit HumanResource(const AgentID id) noexcept : companyBoard_{id} {}

    [[nodiscard]] auto planAndRequestBudget(const HeadCount layOffsCnt) noexcept -> Budget {
        const auto layOffsPlan = std::min(employeeCnt(), layOffsCnt);
        layOffsPlan_           = layOffsPlan;
        const auto wageSum     = sumWage();
        const auto avgWage     = wageSum.value() / employeeCnt().value();
        requestedBudget_       = static_cast<Budget>((employeeCnt() - layOffsPlan) * Wage{avgWage});
        return *requestedBudget_;
    }

    void revisePlan(const Budget budget) noexcept {
        ASSERT(requestedBudget_);
        ASSERT(budget <= requestedBudget_);
        if (budget == requestedBudget_) return;
        const auto cutSumWage = sumWage().value() - budget.value();
        if (cutSumWage <= 0.0) return;
        const auto avgWage = sumWage() / employeeCnt().value();
        const auto layOffs = cutSumWage / avgWage.value();
        layOffsPlan_.emplace(layOffs);
    }

    [[nodiscard]] auto requestedBudget() const noexcept -> Budget {
        ASSERT(requestedBudget_);
        return *requestedBudget_;
    }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount { return roster_.employeeCnt(); }

    [[nodiscard]] auto sumWage() const noexcept -> Wage {
        auto       entries = roster_.validEntries();
        const auto out     = std::ranges::fold_left(
            entries | std::views::transform(&RosterEntry::wage), Wage{0.0}, std::plus{}
        );
        ASSERT(out >= Wage{0.0});
        return out;
    }

    [[nodiscard]] auto addRoster(
        const AgentID id, const Wage wage, base_goods::Workspace& workspace
    ) noexcept -> RosterEntry& {
        ASSERT(wage.isPositive());
        return roster_.add(id, wage, companyBoard_, workspace);
    }

    void layOffs() noexcept {
        const auto layOffsCnt = layOffsPlan_;
        ASSERT(layOffsCnt->isZeroOrMore());

        auto currentLayOffs = HeadCount{0.0};
        for (auto& entry : roster_.rawEntries()) {
            if (currentLayOffs >= layOffsCnt) break;
            if (not entry.isOccupied()) continue;
            entry.resign();
            ++currentLayOffs;
        }
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