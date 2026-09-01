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

  protected:
    explicit BaseFirm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : finance{Id, masterRng},
          laborDemander{masterRng, {Id}},
          capitalDemander{masterRng},
          id{Id} {}
};

class GoodsFirm final : public BaseFirm {
  public:
    explicit GoodsFirm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : BaseFirm(Id, masterRng), goodsSupplier{masterRng} {}

    GoodsSupplier goodsSupplier;

  private:
    static inline constinit int nextBusinessDay{};
};

class CapitalFirm final : public BaseFirm {
  public:
    explicit CapitalFirm(const AgentID Id, RandomGenerator& masterRng) noexcept
        : BaseFirm(Id, masterRng), capitalSupplier{masterRng} {}

    CapitalSupplier capitalSupplier;

  private:
    static inline constinit int nextBusinessDay{};
};
}  // namespace abm