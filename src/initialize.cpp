#include "core/engine.hpp"

#include <cstdlib>
#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>
#include <iostream>

#include "config.hpp"
#include "util.hpp"
#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm {
Engine::Engine(const int totalStep) noexcept
    : totalStep_{totalStep},
      seed_{generateSeed()},
      rng_{pcg32{seed_.state, seed_.stream}},
      laborMarket_{rng_},
      productionGoodsMarket_{rng_},
      consumerGoodsMarket_{rng_} {
    if (not logger_.isValid()) {
        std::cerr << "can not create file\n";
        std::abort();
    }

    namespace cnt = config::agent_count;

    bToCFirms_.reserve(cnt::bToCFirm);
    int agentId{};
    for (; agentId < config::agent_count::bToCFirm; ++agentId) {
    }

    bToBFirms_.reserve(cnt::bToBFirm);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm; ++agentId) {
    }

    hholds_.reserve(cnt::hhold);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm + cnt::hhold; ++agentId) {
    }
}

Logger::Logger()
    : file_{[]() -> HighFive::File {
          const auto filepath =
              static_cast<std::string>(config::setting::simulationResultOutputPath);
          const auto path = std::filesystem::path{filepath};
          if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
          return HighFive::File{
              filepath,
              HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
          };
      }()} {}
}  // namespace abm