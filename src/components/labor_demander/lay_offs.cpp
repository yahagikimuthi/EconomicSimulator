#include "components/labor_demander.hpp"

#include <ranges>

#include "core/values/labor.hpp"

namespace labor_demander {
void HumanResourceManager::layOffs(const HeadCount layOffsCnt) {
    HeadCount              currentLayOffsCnt{0.0};
    auto&                  roster = companyBoard_.roster;
    std::ranges::view auto alignedEntries{
        roster | std::views::filter(&world::RosterEntry::isOccupied) | std::views::reverse |
        std::views::take(layOffsCnt.value())
    };
    for (auto&& entry : alignedEntries) {
        entry.isOccupied = false;
        emptyRosterPool_.emplace_back(&entry);
    }
}
}  // namespace labor_demander