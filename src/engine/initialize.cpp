#include "core/engine.hpp"

#include <entt/entt.hpp>

#include "components/common.hpp"
#include "components/labor_demander.hpp"
#include "config/init_setup.hpp"

namespace core {
Engine::Engine(const int totalStep) : totalStep_{totalStep} {
    for (int i{}; i < config::agent_count::firm; ++i) {
        entt::entity firm = registry_.create();
        registry_.emplace<agent_index::Component>(firm, i);
        registry_.emplace<labor_demander::Component>(firm);
    }
}
}  // namespace core