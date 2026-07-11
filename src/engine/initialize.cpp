#include "core/engine.hpp"

#include <cassert>
#include <entt/entt.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>
#include <ranges>
#include <string>

#include "config.hpp"

namespace core {
/*
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
*/
Engine::Engine(const int totalStep, const std::string& filename)
    : logger{filename}, totalStep_{totalStep} {
    if (not logger.isValid()) {
        assert(false && "can not create file");
    }

    firms_.reserve(config::agent_count::firm);
    for (const auto _ : std::views::iota(0, config::agent_count::firm)) firms_.emplace_back();

    hholds_.reserve(config::agent_count::hhold);
    for (const auto _ : std::views::iota(0, config::agent_count::hhold)) hholds_.emplace_back();
}

Logger::Logger(const std::string& filename)
    : file_{
          filename, HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
      } {}
}  // namespace core