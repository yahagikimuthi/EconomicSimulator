#pragma once

#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "util.hpp"

namespace abm::consumer_goods::supplier {
class ConsumerGoodsSupplierFactory {
  public:
    [[nodiscard]] static auto create(RandomGenerator& masterRng) noexcept -> ConsumerGoodsSupplier {
        auto supplier = ConsumerGoodsSupplier{masterRng};
        return supplier;
    }
};
}  // namespace abm::consumer_goods::supplier