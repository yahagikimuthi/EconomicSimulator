#include "components/labor_demander.hpp"

namespace labor_demander {
void HumanResourceManager::layOffs(const int layOffsCnt) {
    int   currentLayOffsCnt{};
    auto& roster = companyBoard_.roster;
    for (int i{static_cast<int>(roster.size()) - 1}; i >= 0; --i) {
        if (currentLayOffsCnt >= layOffsCnt) break;
        auto& rosterEntry = roster[static_cast<std::size_t>(i)];
        if (not rosterEntry.isOccupied) continue;
        emptyRosterPool_.emplace_back(&rosterEntry);
        ++currentLayOffsCnt;
    }
}
}  // namespace labor_demander