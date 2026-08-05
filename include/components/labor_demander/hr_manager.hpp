#pragma once

#include <cstddef>
#include <ranges>
#include <vector>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
class [[nodiscard]] HumanResourceManager {
  public:
    HumanResourceManager(world::CompanyBoard& companyBoard) : companyBoard_{companyBoard} {}
    auto addRoster(const AgentID id, const Wage wage, world::Workspace& workspace)
        -> SafePtr<world::RosterEntry> PRE(id >= AgentID{0}) PRE(wage > Wage{0.0});
    void acceptResignation();
    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0});
    auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        ASSERT(companyBoard_.roster.size() >= emptyRosterPool_.size());
        const std::size_t rosterSize{companyBoard_.roster.size() - emptyRosterPool_.size()};
        return HeadCount{static_cast<double>(rosterSize)};
    }
    auto sumWage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        using Entry        = world::RosterEntry;
        const auto& roster = companyBoard_.roster;
        return Wage{std::ranges::fold_left(
            roster | std::views::filter([](const Entry& entry) -> bool {
                return entry.isOccupied;
            }) | std::views::transform([](const Entry& entry) -> double {
                return entry.wage.value();
            }),
            0.0,
            std::plus{}
        )};
    };
    void endStep() { companyBoard_.resignationBox.clear(); }

  private:
    world::CompanyBoard&                     companyBoard_;
    std::vector<SafePtr<world::RosterEntry>> emptyRosterPool_;
};
}  // namespace labor::demander