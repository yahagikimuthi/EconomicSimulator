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
    static inline constinit int agentCnt_{};

  public:
    const AgentID id{agentCnt_++};
};

struct CapitalFirm : Agent {
    explicit CapitalFirm(RandomGenerator& masterRng) noexcept
        : finance{id, masterRng},
          laborDemander{id, masterRng},
          capitalDemander{masterRng},
          capitalSupplier{masterRng} {}

    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    CapitalSupplier capitalSupplier;
};

struct GoodsFirm : Agent {
    explicit GoodsFirm(RandomGenerator& masterRng) noexcept
        : finance{id, masterRng},
          laborDemander{id, masterRng},
          capitalDemander{masterRng},
          goodsSupplier{masterRng} {}

    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    GoodsSupplier   goodsSupplier;
};

struct HHold : Agent {
    explicit HHold(RandomGenerator& masterRng) noexcept
        : finance{id, masterRng}, labor{masterRng}, goods{masterRng} {}

    HHoldFinance  finance;
    LaborSupplier labor;
    GoodsDemander goods;
};
}  // namespace abm