#include "components/labor_demander.hpp"

#include <ranges>

#include "core/values/labor.hpp"

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
}  // namespace labor::demander