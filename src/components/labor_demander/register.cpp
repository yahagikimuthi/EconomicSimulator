#include "components/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <memory>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
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
}  // namespace labor::demander