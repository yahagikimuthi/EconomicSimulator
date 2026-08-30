#pragma once

#include "components/consumer_goods_demander.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/util.hpp"

namespace abm {
class HHold final {
  public:
    [[nodiscard]] explicit HHold(RandomGenerator& masterRng) noexcept;

  private:
    finance::Finance      finance_;
    LaborSupplier         laborSupplier_;
    ConsumerGoodsDemander consumerGoodsDemander_;
};
}  // namespace abm