#include "core/engine.hpp"

#include <cstdlib>
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

    namespace cnt = setting::agent_count;

    bToCFirms_.reserve(cnt::bToCFirm);
    int agentId{};
    for (; agentId < cnt::bToCFirm; ++agentId) {
    }

    bToBFirms_.reserve(cnt::bToBFirm);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm; ++agentId) {
    }

    hholds_.reserve(cnt::hhold);
    for (; agentId < cnt::bToCFirm + cnt::bToBFirm + cnt::hhold; ++agentId) {
    }
}
}  // namespace abm