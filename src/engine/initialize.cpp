#include "core/engine.hpp"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>
#include <iostream>
#include <ranges>

#include "config.hpp"
#include "helper.hpp"

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
        std::cerr << "can not create file\n";
        std::abort();
    }
    masterRng_ = {seed_.state, seed_.stream};
    firms_.reserve(config::agent_count::firm);
    workspaces_.resize(config::agent_count::firm);
    for (const auto i : std::views::iota(0, config::agent_count::firm)) {
        companyBoards_.emplace_back(i);
        firms_.emplace_back(Firm{
            .index   = {i},
            .finance = {masterRng_},
            .labor   = {masterRng_, companyBoards_[static_cast<std::size_t>(i)]},
            .goods   = {masterRng_, workspaces_[static_cast<std::size_t>(i)]}
        });
    }

    hholds_.reserve(config::agent_count::hhold);
    for (const auto i : std::views::iota(0, config::agent_count::hhold)) {
        HHold hhold{
            .index = {i}, .finance = {masterRng_}, .labor = {masterRng_}, .goods = {masterRng_}
        };
        hholds_.push_back(hhold);
    }
}

Logger::Logger()
    : file_{[]() -> HighFive::File {
          const std::string filepath{
              static_cast<std::string>(config::setting::simulationResultOutputPath)
          };
          const std::filesystem::path path{filepath};
          if (path.has_parent_path()) {
              std::filesystem::create_directories(path.parent_path());
          }
          return HighFive::File{
              filepath,
              HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
          };
      }()} {}
}  // namespace core