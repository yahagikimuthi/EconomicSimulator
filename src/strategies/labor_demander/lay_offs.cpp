#include "strategies/labor_demander/lay_offs.hpp"

namespace labor_demander {
void layoffs(LayOffsView view, const int layOffsCnt) {
    int currentLayOffsCnt{};
    for (int i{view.rosterSize() - 1}; i >= 0; --i) {
        if (currentLayOffsCnt >= layOffsCnt) break;
        auto& rosterEntry = view.getRosterEntry(i);
        if (not rosterEntry.isOccupied_) continue;
        rosterEntry.isOccupied_ = false;
        view.addEmptyRoster(rosterEntry);
        ++currentLayOffsCnt;
    }
}
}  // namespace labor_demander