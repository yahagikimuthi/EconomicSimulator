#pragma once

#include "agents/common.hpp"
#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"

namespace abm {
class CapitalFirm final {
    static inline constinit int instanceCnt{};

  public:
    CapitalFirm(const AgentID id, RandomGenerator& masterRng) noexcept;

    void act(const Date& today, Markets& markets) noexcept;

  private:
    FirmFinance     finance_;
    LaborDemander   laborDemander_;
    CapitalDemander capitalDemander_;
    CapitalSupplier capitalSupplier_;
    const AgentID   id_;
    const int       businessDay_;
};
}  // namespace abm