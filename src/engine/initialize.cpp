#include "core/engine.hpp"

#include <entt/entt.hpp>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "config/init_setup.hpp"

namespace core {

namespace {
void createFirm(const int id, entt::registry& registry) {
    entt::entity firm{registry.create()};
    registry.emplace<agent_index::Component>(firm, id);
    registry.emplace<firm_finance::Component>(firm);
    registry.emplace<labor_demander::Component>(firm);
    registry.emplace<goods_supplier::Component>(firm);
    registry.emplace<FirmTag>(firm);
}

void createHHold(const int id, entt::registry& registry) {
    entt::entity hhold{registry.create()};
    registry.emplace<agent_index::Component>(hhold, id);
    registry.emplace<hhold_finance::Component>(hhold);
    registry.emplace<labor_supplier::Component>(hhold);
    registry.emplace<goods_demander::Component>(hhold);
    registry.emplace<HHoldTag>(hhold);
}
}  // namespace

Engine::Engine(const int totalStep) : totalStep_{totalStep} {
    for (int i{}; i < config::agent_count::firm; ++i) createFirm(i, registry_);
    for (int i{}; i < config::agent_count::hhold; ++i) createHHold(i, registry_);
}
}  // namespace core