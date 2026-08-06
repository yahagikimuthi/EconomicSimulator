#include "components/labor_demander/hr_manager.hpp"

#include <memory>
#include <numeric>
#include <ranges>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
void HumanResourceManager::layOffs(const HeadCount layOffsCnt) {
    HeadCount              currentLayOffsCnt{0.0};
    auto&                  roster = companyBoard_.roster;
    std::ranges::view auto reversedRoster{roster | std::views::reverse};

    HeadCount currentLayOffs{0.0};
    for (auto&& entry : reversedRoster) {
        if (currentLayOffs >= layOffsCnt) return;
        if (not entry.isOccupied) continue;
        entry.isOccupied = false;
        emptyRosterPool_.emplace_back(&entry);
        ++currentLayOffs;
    }
}

auto HumanResourceManager::addRoster(const AgentID id, const Wage wage, world::Workspace& workspace)
    -> SafePtr<world::RosterEntry> {
    if (emptyRosterPool_.empty())
        return &companyBoard_.roster.emplace_back(id, wage, companyBoard_, workspace);
    world::RosterEntry* newRoster = emptyRosterPool_.back().get();
    std::destroy_at(newRoster);
    std::construct_at(newRoster, id, wage, companyBoard_, workspace);
    emptyRosterPool_.pop_back();
    return newRoster;
}

void HumanResourceManager::acceptResignation() {
    auto& resignationBox = companyBoard_.resignationBox;
    for (const SafePtr<world::RosterEntry> resignEntry : resignationBox) {
        resignEntry->isOccupied = false;
        emptyRosterPool_.emplace_back(resignEntry);
    }
}

auto HumanResourceManager::sumWage() const -> Wage POST(wage : wage >= Wage{0.0}) {
    using Entry                   = world::RosterEntry;
    const auto&            roster = companyBoard_.roster;
    std::ranges::view auto wages{
        roster | std::views::filter(&Entry::isOccupied) |
        std::views::transform([](const Entry& entry) -> double { return entry.wage.value(); })
    };
    return Wage{std::reduce(wages.begin(), wages.end())};
};
}  // namespace labor::demander