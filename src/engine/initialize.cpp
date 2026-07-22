#include "core/engine.hpp"

#include <cassert>
#include <entt/entt.hpp>
#include <helper.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>
#include <ranges>

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
Engine::Engine(const int totalStep) : totalStep_{totalStep}, seed_{helper::generatePCG32Seed()} {
    if (not logger_.isValid()) {
        assert(false && "can not create file");
    }
    masterRng_ = {seed_.state, seed_.stream};

    firms_.reserve(config::agent_count::firm);
    workspaces_.resize(config::agent_count::firm);
    for (const auto i : std::views::iota(0, config::agent_count::firm + 1)) {
        companyBoards_.emplace_back(i);
        Firm firm{
            .index   = {i},
            .finance = {makeSeed(), makeSeed()},
            .labor   = {makeSeed(), makeSeed(), &companyBoards_[static_cast<std::size_t>(i)]},
            .goods   = {makeSeed(), makeSeed(), &workspaces_[static_cast<std::size_t>(i)]}
        };
        firms_.push_back(firm);
    }

    hholds_.reserve(config::agent_count::hhold);
    for (const auto i : std::views::iota(0, config::agent_count::hhold)) {
        HHold hhold{
            .index   = {i},
            .finance = {makeSeed(), makeSeed()},
            .labor   = {makeSeed(), makeSeed()},
            .goods   = {makeSeed(), makeSeed()}
        };
        hholds_.push_back(hhold);
    }
}

Logger::Logger()
    : file_{
          static_cast<std::string>(config::setting::simulationResultOutputPath),
          HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
      } {}
}  // namespace core