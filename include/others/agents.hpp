#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/util.hpp"

namespace abm {
struct Agent {
  private:
    static inline constinit int agentCnt{};

  public:
    const AgentID id{agentCnt++};
};

struct CapitalFirm : Agent {
    explicit CapitalFirm(RandomGenerator& masterRng) noexcept;

    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    CapitalSupplier capitalSupplier;
};

struct GoodsFirm : Agent {
    explicit GoodsFirm(RandomGenerator& masterRg) noexcept;

    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    GoodsSupplier   goodsSupplier;
};

struct HHold : Agent {
    explicit HHold(RandomGenerator& masterRng) noexcept;

    HHoldFinance  finance;
    LaborSupplier labor;
    GoodsDemander goods;
};
}  // namespace abm