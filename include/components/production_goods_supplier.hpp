#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "world/message.hpp"

namespace production_goods::supplier {
class [[nodiscard]] ProductionGoodsSupplier
    : public base_goods::supplier::BaseGoodsSupplier<world::ProductionGoodsEntry> {
  public:
    using base_goods::supplier::BaseGoodsSupplier<world::ProductionGoodsEntry>::BaseGoodsSupplier;
};
}  // namespace production_goods::supplier