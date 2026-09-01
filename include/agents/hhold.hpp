#pragma once

#include "agents/common.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"

namespace abm {
class HHold final {
  public:
    HHold(const AgentID id, RandomGenerator& masterRng) noexcept
        : finance_{id, masterRng}, laborSupplier_{masterRng}, goodsDemander_{masterRng}, id_{id} {}

    void act(const Date today, Markets& markets) noexcept;

  private:
    HHoldFinance  finance_;
    LaborSupplier laborSupplier_;
    GoodsDemander goodsDemander_;
    const AgentID id_;
};
}  // namespace abm