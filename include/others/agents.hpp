#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class HHold final {
  public:
    explicit HHold(const AgentID Id, RandomGenerator& masterRng) noexcept
        : finance{Id, masterRng}, laborSupplier{masterRng}, goodsDemander{masterRng}, id{Id} {}

    HHoldFinance  finance;
    LaborSupplier laborSupplier;
    GoodsDemander goodsDemander;
    const AgentID id;
};

class Firm final {
  public:
    explicit Firm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : finance{Id, masterRng},
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