#pragma once

#include "values/date.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm {
struct Markets final {
    CapitalMarket capitalMarket;
    GoodsMarket   goodsMarket;
    LaborMarket   laborMarket;
};

template <typename T>
concept IAgent = requires(T t, const Date& date, Markets& markets) { t.act(date, markets); };
}  // namespace abm