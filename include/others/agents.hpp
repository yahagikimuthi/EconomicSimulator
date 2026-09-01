#pragma once

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier/goods_supplier.hpp"
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

class BaseFirm {
  public:
    FirmFinance     finance;
    LaborDemander   laborDemander;
    CapitalDemander capitalDemander;
    const AgentID   id;
    const int       businessDay;

  protected:
    explicit BaseFirm(const AgentID Id, const int BusinessDay, RandomGenerator& masterRng) noexcept
        : finance{Id, masterRng},
          laborDemander{Id, masterRng},
          capitalDemander{masterRng},
          id{Id},
          businessDay{BusinessDay} {}
};

class GoodsFirm final : public BaseFirm {
  public:
    explicit GoodsFirm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : BaseFirm(Id, instanceCnt++, masterRng), goodsSupplier{masterRng} {}

    GoodsSupplier goodsSupplier;

  private:
    static inline constinit int instanceCnt{};
};

class CapitalFirm final : public BaseFirm {
  public:
    explicit CapitalFirm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : BaseFirm(Id, instanceCnt++, masterRng), capitalSupplier{masterRng} {}

    CapitalSupplier capitalSupplier;

  private:
    static inline constinit int instanceCnt{};
};
}  // namespace abm