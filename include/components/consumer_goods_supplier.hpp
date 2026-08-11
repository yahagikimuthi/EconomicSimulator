#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] ConsumerGoodsSupplier : public BaseGoodsSupplier<Market::consumerGoods> {
  public:
    using BaseGoodsSupplier<Market::consumerGoods>::BaseGoodsSupplier;
};
}  // namespace abm