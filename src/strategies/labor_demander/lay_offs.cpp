#include "strategies/labor_demander.hpp"

namespace labor_demander {
void layoffs(
    LayOffsView view, const int layOffsCnt, std::vector<world::CompanyBoard>& companyBoards
) {
    auto& roster          = view.roster(companyBoards);
    auto& emptyRosterPool = view.emptyRosterPool();
    if (roster.empty()) return;

    int currentLayOffsCnt{};
    for (int i{static_cast<int>(roster.size()) - 1}; i >= 0; --i) {
        if (currentLayOffsCnt >= layOffsCnt) break;

        auto& rosterEntry = roster[static_cast<std::size_t>(i)];
        if (not rosterEntry.isOccupied_) continue;
        rosterEntry.isOccupied_ = false;
        emptyRosterPool.emplace_back(static_cast<std::size_t>(i));
        ++currentLayOffsCnt;
    }
}
}  // namespace labor_demander