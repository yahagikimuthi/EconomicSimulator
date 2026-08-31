#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander::human_resource {
class EmptyRosterPool final {
  public:
    [[nodiscard]] EmptyRosterPool() noexcept = default;
    [[nodiscard]] auto size() const noexcept -> std::size_t { return pool_.size(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return size() == 0UZ; }

    auto popBackEntry() noexcept -> RosterEntry& {
        ASSERT(not empty());
        auto& back = pool_.back().get();
        ASSERT(not back.isOccupied());
        pool_.pop_back();
        return back;
    }
    void add(RosterEntry& entry) noexcept {
        ASSERT(not entry.isOccupied());
        pool_.emplace_back(std::ref(entry));
    }

  private:
    std::vector<RefWrap<RosterEntry>> pool_;
};

class HumanResource final {
  public:
    [[nodiscard]] explicit HumanResource(CompanyBoard&& companyBoard) noexcept
        : companyBoard_{std::move(companyBoard)} {}

    [[nodiscard]] auto planAndRequestBudget(const HeadCount layOffsCnt) noexcept -> Money {
        const auto layOffsPlan = std::min(employeeCnt(), layOffsCnt);
        layOffsPlan_           = layOffsPlan;
        const auto wageSum     = sumWage();
        const auto avgWage     = wageSum.value() / employeeCnt().value();
        requestedBudget_       = (employeeCnt() - layOffsPlan) * Wage{avgWage};
        return *requestedBudget_;
    }

    void revisePlan(const Money budget) noexcept {
        ASSERT(requestedBudget_);
        ASSERT(budget <= requestedBudget_);
        if (budget == requestedBudget_) return;
        const auto cutSumWage = sumWage().value() - budget.value();
        if (cutSumWage <= 0.0) return;
        const auto avgWage = sumWage() / employeeCnt().value();
        const auto layOffs = cutSumWage / avgWage.value();
        layOffsPlan_.emplace(layOffs);
    }

    [[nodiscard]] auto requestedBudget() const noexcept -> Money {
        ASSERT(requestedBudget_);
        return *requestedBudget_;
    }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount {
        ASSERT(companyBoard_.roster.size() >= emptyRosterPool_.size());
        const auto rosterSize = companyBoard_.roster.size() - emptyRosterPool_.size();
        const auto out        = HeadCount{rosterSize};
        ASSERT(out >= HeadCount{0.0});
        return out;
    }

    [[nodiscard]] auto sumWage() const noexcept -> Wage {
        const auto& roster = companyBoard_.roster;

        auto wages = roster | std::views::filter(&RosterEntry::isOccupied) |
                     std::views::transform([](const RosterEntry& e) noexcept -> double {
                         return e.wage.value();
                     });
        const auto sumWage = std::ranges::fold_left(wages, 0.0, std::plus{});
        ASSERT(sumWage >= 0.0);
        return Wage{sumWage};
    }

    [[nodiscard]] auto addRoster(
        const AgentID id, const Wage wage, base_goods::Workspace& workspace
    ) noexcept -> RosterEntry& {
        ASSERT(wage > Wage{0.0});

        if (emptyRosterPool_.empty()) return companyBoard_.addRoster(id, wage, workspace);
        auto* newRoster = &emptyRosterPool_.popBackEntry();
        ASSERT(newRoster != nullptr);
        std::destroy_at(newRoster);
        std::construct_at(newRoster, id, wage, companyBoard_, workspace);
        return *newRoster;
    }

    void acceptResignation() noexcept {
        auto& resignationBox = companyBoard_.resignationBox;
        for (RosterEntry& resignEntry : resignationBox) {
            ASSERT(resignEntry.isOccupied());
            resignEntry.disable();
            emptyRosterPool_.add(resignEntry);
        }
        resignationBox.clear();
    }

    void layOffs() noexcept {
        const auto layOffsCnt = layOffsPlan_;
        ASSERT(layOffsCnt >= HeadCount{0.0});

        auto currentLayOffs = HeadCount{0.0};
        for (auto& entry : companyBoard_.roster) {
            if (currentLayOffs >= layOffsCnt) break;
            if (not entry.isOccupied()) continue;
            entry.disable();
            emptyRosterPool_.add(entry);
            ++currentLayOffs;
        }
    }

  private:
    CompanyBoard             companyBoard_;
    EmptyRosterPool          emptyRosterPool_;
    std::optional<HeadCount> layOffsPlan_;
    std::optional<Money>     requestedBudget_;
};
}  // namespace abm::labor::demander::human_resource

namespace abm::labor::demander {
using HumanResource = human_resource::HumanResource;
}