#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class HHold final {
  public:
    explicit HHold(const AgentID Id, RandomGenerator& masterRng) noexcept
        : finance{masterRng}, laborSupplier{masterRng}, consumerGoodsDemander{masterRng}, id{Id} {}

    FirmFinance           finance;
    LaborSupplier         laborSupplier;
    ConsumerGoodsDemander consumerGoodsDemander;
    const AgentID         id;
};

class Firm final {
  public:
    explicit Firm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : finance{masterRng},
          laborDemander{masterRng, {Id}},
          capitalDemander{masterRng},
          goodsSupplier{masterRng},
          id{Id} {}

    FirmFinance       finance;
    LaborDemander     laborDemander;
    CapitalDemander   capitalDemander;
    BaseGoodsSupplier goodsSupplier;
    const AgentID     id;
};
}  // namespace abm