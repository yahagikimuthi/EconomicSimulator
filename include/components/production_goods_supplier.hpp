#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] ProductionGoodsSupplier : public BaseGoodsSupplier<Market::productionGoods> {
  public:
    using BaseGoodsSupplier<Market::productionGoods>::BaseGoodsSupplier;
};
}  // namespace abm