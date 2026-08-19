#pragma once

#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_demander/mediator.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
class Factory {
    static auto create(RandomGenerator& masterRng, const AgentID id, const Market firmType)
        -> LaborDemander {
        auto board    = CompanyBoard{id, firmType};
        auto demander = LaborDemander{masterRng, std::move(board)};
        demander.mediator_.employPlanListeners_[0] =
            &demander.recruitSystem_.planner_.wagePlanner_.memory_;
        auto& resultListeners = demander.mediator_.recruitResultListeners_;
        resultListeners[0]    = &demander.recruitSystem_.planner_.wagePlanner_.memory_;
        resultListeners[1] =
            &demander.recruitSystem_.planner_.employPlanSystem_.offerPlanner_.memory_;
        return demander;
    }
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemanderFactory = labor::demander::Factory;
}