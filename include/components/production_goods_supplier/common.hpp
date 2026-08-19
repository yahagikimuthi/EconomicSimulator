#pragma once

#include "components/base_goods_supplier/common.hpp"

namespace abm::production_goods::supplier {
template <typename T>
concept IMediator = base_goods::supplier::IMediator<T>;
}