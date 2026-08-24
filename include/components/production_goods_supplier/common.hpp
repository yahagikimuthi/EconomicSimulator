#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "world/goods.hpp"

namespace abm::production_goods::supplier {
template <typename T>
concept IMediator = base_goods::supplier::IMediator<T>;

using Market  = ProductionGoodsMarket;
using Entry   = ProductionGoodsEntry;
using Request = ProductionGoodsRequest;
}  // namespace abm::production_goods::supplier