#pragma once

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
}  // namespace abm