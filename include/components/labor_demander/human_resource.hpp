#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/goods.hpp"
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
        ASSERT(not back.isOccupied);
        pool_.pop_back();
        return back;
    }
    void add(RosterEntry& entry) noexcept {
        ASSERT(not entry.isOccupied);
        pool_.emplace_back(std::ref(entry));
    }

  private:
    std::vector<RefWrap<RosterEntry>> pool_;
};

class HumanResource final {
  public:
    [[nodiscard]] explicit HumanResource(CompanyBoard&& companyBoard) noexcept
        : companyBoard_{std::move(companyBoard)}, sumWage_{[&]() -> Wage {
              const auto& roster = companyBoard_.roster;

              auto wages = roster | std::views::filter(&RosterEntry::isOccupied) |
                           std::views::transform(&RosterEntry::wage) |
                           std::views::transform([](Wage wage) -> double { return wage.value(); });
              return Wage{std::ranges::fold_left(wages, 0.0, std::plus{})};
          }()} {}

    [[nodiscard]] auto addRoster(const AgentID id, const Wage wage, Workspace& workspace) noexcept
        -> RosterEntry& PRE(id >= AgentID{0}) PRE(wage > Wage{0.0}) {
        sumWage_ += wage;
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
            ASSERT(resignEntry.isOccupied);
            resignEntry.isOccupied = false;
            emptyRosterPool_.add(resignEntry);
            sumWage_ -= resignEntry.wage;
        }
        resignationBox.clear();
    }

    void layOffs(const HeadCount layOffsCnt) noexcept {
        ASSERT(layOffsCnt >= HeadCount{0.0});

        auto currentLayOffs = HeadCount{0.0};
        for (auto& entry : companyBoard_.roster) {
            if (currentLayOffs >= layOffsCnt) break;
            if (not entry.isOccupied) continue;
            entry.isOccupied = false;
            emptyRosterPool_.add(entry);
            ++currentLayOffs;
            sumWage_ -= entry.wage;
        }
    }

    [[nodiscard]] auto employeeCnt() const noexcept -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        ASSERT(companyBoard_.roster.size() >= emptyRosterPool_.size());
        const auto rosterSize = std::size_t{companyBoard_.roster.size() - emptyRosterPool_.size()};
        return HeadCount{static_cast<double>(rosterSize)};
    }

    [[nodiscard]] auto sumWage() const noexcept -> Wage {
        ASSERT(sumWage_ >= Wage{0.0});
        return sumWage_;
    }

  private:
    CompanyBoard    companyBoard_;
    EmptyRosterPool emptyRosterPool_;
    Wage            sumWage_;
};
}  // namespace abm::labor::demander::human_resource

namespace abm::labor::demander {
using HumanResource = human_resource::HumanResource;
}