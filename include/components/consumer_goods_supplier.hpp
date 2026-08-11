#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "world/message.hpp"

namespace consumer_goods::supplier {
class [[nodiscard]] ConsumerGoodsSupplier
    : public base_goods::supplier::BaseGoodsSupplier<Market::consumerGoods> {
  public:
    using base_goods::supplier::BaseGoodsSupplier<Market::consumerGoods>::BaseGoodsSupplier;
};
}  // namespace consumer_goods::supplier