#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "world/consumer_goods.hpp"

namespace abm::consumer_goods::supplier {
template <typename T>
concept IMediator = base_goods::supplier::IMediator<T>;

using Market  = ConsumerGoodsMarket;
using Entry   = ConsumerGoodsEntry;
using Request = ConsumerGoodsRequest;
}  // namespace abm::consumer_goods::supplier