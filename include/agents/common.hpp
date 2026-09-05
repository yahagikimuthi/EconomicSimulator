#pragma once

#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/date.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm {
struct MarketRegistry final {
    explicit MarketRegistry(const Date& today) noexcept
        : capitalMarket{today}, goodsMarket{today} {}

    CapitalMarket capitalMarket;
    GoodsMarket   goodsMarket;
    LaborMarket   laborMarket;
};

template <typename T>
concept IAgent = requires(T t, const Date& date, MarketRegistry& markets) { t.act(date, markets); };

class Agent {
    static inline constinit int agentCnt{};

  protected:
    explicit Agent() noexcept {
        ASSERT(Day{2} <= operationDay_ and operationDay_ <= Day{global_setting::dayInMonth - 1});
    }

    const AgentID id_{agentCnt};
    const Day     operationDay_{(agentCnt++ % (global_setting::dayInMonth - 2)) + 2};
};
}  // namespace abm