#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "world/capital.hpp"

namespace abm::capital::supplier {
template <typename T>
concept IMediator = base_goods::supplier::IMediator<T>;

using Market  = CapitalMarket;
using Entry   = CapitalEntry;
using Request = CapitalRequest;
}  // namespace abm::capital::supplier