#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <ranges>
#include <vector>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
class [[nodiscard]] HumanResourceManager {
  public:
    HumanResourceManager(CompanyBoard&& companyBoard) : companyBoard_{std::move(companyBoard)} {}
    auto addRoster(const AgentID id, const Wage wage, Workspace& workspace)
        -> RosterEntry& PRE(id >= AgentID{0}) PRE(wage > Wage{0.0}) {
        if (emptyRosterPool_.empty()) return companyBoard_.addRoster(id, wage, workspace);
        RosterEntry* newRoster = &emptyRosterPool_.back().get();
        ASSERT(newRoster != nullptr);
        std::destroy_at(newRoster);
        std::construct_at(newRoster, id, wage, companyBoard_, workspace);
        emptyRosterPool_.pop_back();
        return *newRoster;
    }

    void acceptResignation() {
        auto& resignationBox = companyBoard_.resignationBox;
        for (RosterEntry& resignEntry : resignationBox) {
            ASSERT(resignEntry.isOccupied);
            resignEntry.isOccupied = false;
            emptyRosterPool_.emplace_back(resignEntry);
        }
        resignationBox.clear();
    }

    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt >= HeadCount{0.0}) {
        HeadCount currentLayOffs{0.0};
        for (auto& entry : companyBoard_.roster) {
            if (currentLayOffs >= layOffsCnt) break;
            if (not entry.isOccupied) continue;
            entry.isOccupied = false;
            emptyRosterPool_.emplace_back(std::ref(entry));
            ++currentLayOffs;
        }
    }

    auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        ASSERT(companyBoard_.roster.size() >= emptyRosterPool_.size());
        const std::size_t rosterSize{companyBoard_.roster.size() - emptyRosterPool_.size()};
        return HeadCount{static_cast<double>(rosterSize)};
    }

    auto sumWage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        using Entry = RosterEntry;
        std::ranges::view auto wages{
            companyBoard_.roster | std::views::filter(&Entry::isOccupied) |
            std::views::transform([](const Entry& entry) -> double { return entry.wage.value(); })
        };
        return Wage{std::reduce(wages.begin(), wages.end())};
    }

    void endStep() { companyBoard_.resignationBox.clear(); }

  private:
    template <typename T>
    using RefWrapper = std::reference_wrapper<T>;
    CompanyBoard                         companyBoard_;
    std::vector<RefWrapper<RosterEntry>> emptyRosterPool_;
};
}  // namespace labor::demander