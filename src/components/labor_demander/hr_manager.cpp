#include "components/labor_demander/hr_manager.hpp"

#include <functional>
#include <memory>
#include <numeric>
#include <ranges>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
void HumanResourceManager::layOffs(const HeadCount layOffsCnt) {
    HeadCount currentLayOffs{0.0};
    for (auto& entry : companyBoard_.roster) {
        if (currentLayOffs >= layOffsCnt) break;
        if (not entry.isOccupied) continue;
        entry.isOccupied = false;
        emptyRosterPool_.emplace_back(std::ref(entry));
        ++currentLayOffs;
    }
}

auto HumanResourceManager::addRoster(const AgentID id, const Wage wage, world::Workspace& workspace)
    -> world::RosterEntry& {
    if (emptyRosterPool_.empty()) return companyBoard_.addRoster(id, wage, workspace);
    world::RosterEntry* newRoster = &emptyRosterPool_.back().get();
    ASSERT(newRoster != nullptr);
    std::destroy_at(newRoster);
    std::construct_at(newRoster, id, wage, companyBoard_, workspace);
    emptyRosterPool_.pop_back();
    return *newRoster;
}

void HumanResourceManager::acceptResignation() {
    auto& resignationBox = companyBoard_.resignationBox;
    for (world::RosterEntry& resignEntry : resignationBox) {
        ASSERT(not resignEntry.isOccupied);
        resignEntry.isOccupied = false;
        emptyRosterPool_.emplace_back(resignEntry);
    }
    resignationBox.clear();
}

auto HumanResourceManager::sumWage() const -> Wage POST(wage : wage >= Wage{0.0}) {
    using Entry = world::RosterEntry;
    std::ranges::view auto wages{
        companyBoard_.roster | std::views::filter(&Entry::isOccupied) |
        std::views::transform([](const Entry& entry) -> double { return entry.wage.value(); })
    };
    return Wage{std::reduce(wages.begin(), wages.end())};
};
}  // namespace labor::demander