#pragma once

#include "components/capital_demander.hpp"
#include "components/capital_supplier.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier.hpp"
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

class ConsumerFirm final {
  public:
    explicit ConsumerFirm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : finance{masterRng},
          laborDemander{masterRng, {Id}},
          capitalDemander{masterRng},
          consumerGoodsSupplier{masterRng},
          id{Id} {}

    FirmFinance           finance;
    LaborDemander         laborDemander;
    CapitalDemander       capitalDemander;
    ConsumerGoodsSupplier consumerGoodsSupplier;
    const AgentID         id;
};

class CapitalFirm final {
  public:
    explicit CapitalFirm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : finance{masterRng},
          laborDemander{masterRng, {Id}},
          capitalDemander{masterRng},
          capitalSupplier{masterRng},
          id{Id} {}

    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    CapitalSupplier capitalSupplier;
    const AgentID   id;
};
}  // namespace abm