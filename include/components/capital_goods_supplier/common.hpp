#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "world/capital_goods.hpp"

namespace abm::capital_goods::supplier {
template <typename T>
concept IMediator = base_goods::supplier::IMediator<T>;

using Market  = CapitalGoodsMarket;
using Entry   = CapitalGoodsEntry;
using Request = CapitalGoodsRequest;
}  // namespace abm::capital_goods::supplier