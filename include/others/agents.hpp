#pragma once

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class HHold final {
  public:
    explicit HHold(RandomGenerator& masterRng) noexcept;

    FirmFinance           finance;
    LaborSupplier         laborSupplier;
    ConsumerGoodsDemander consumerGoodsDemander;
    const AgentID         id;
};

class ConsumerFirm final {
  public:
    explicit ConsumerFirm(RandomGenerator& masterRng) noexcept;

    FirmFinance           finance;
    LaborDemander         laborDemander;
    CapitalDemander       capitalDemander;
    ConsumerGoodsSupplier consumerGoodsSupplier;
    const AgentID         id;
};

class CapitalFirm final {
  public:
    explicit CapitalFirm(RandomGenerator& masterRng) noexcept;

    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    CapitalSupplier capitalSupplier;
    const AgentID   id;
};
}  // namespace abm