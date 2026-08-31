#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "world/capital.hpp"

namespace abm::capital::supplier {
template <typename T>
concept IMediator = base_goods::supplier::IMediator<T>;

using Market  = Market;
using Entry   = Entry;
using Request = Request;
}  // namespace abm::capital::supplier