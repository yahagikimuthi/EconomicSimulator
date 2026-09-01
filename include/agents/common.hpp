#pragma once

#include "values/date.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm {
struct Markets final {
    CapitalMarket& capitalMarket;
    GoodsMarket&   goodsMarket;
    LaborMarket&   laborMarket;
};

template <typename T>
concept IAgent = requires(T t, const Date& today, Markets& markets) { t.act(today, markets); };
}  // namespace abm