#pragma once

#include "values/common.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm {
struct MarketRegistry final {
    LaborMarket   labor;
    CapitalMarket capital;
    GoodsMarket   goods;
};

class Agent {
    static inline constinit int nextId{};

  protected:
    const AgentID id_{nextId++};
};
}  // namespace abm