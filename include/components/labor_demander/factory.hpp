#pragma once

#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_demander/mediator.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
class LaborDemanderFactory {
  public:
    [[nodiscard]] static auto create(
        RandomGenerator& masterRng, const AgentID id, const Market firmType
    ) noexcept -> LaborDemander {
        auto board    = CompanyBoard{id, firmType};
        auto demander = LaborDemander{masterRng, std::move(board)};
        return demander;
    }
};
}  // namespace abm::labor::demander

namespace abm {
using LaborDemanderFactory = labor::demander::LaborDemanderFactory;
}