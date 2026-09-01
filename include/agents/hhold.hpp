#pragma once

#include "agents/common.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"

namespace abm {
class HHold final {
    static inline constinit int instanceCnt{};

  public:
    HHold(const AgentID id, RandomGenerator& masterRng) noexcept
        : finance_{id, masterRng}, laborSupplier_{masterRng}, goodsDemander_{masterRng}, id_{id} {}

    void act(const Date& date, Markets& markets) noexcept;

  private:
    HHoldFinance  finance_;
    LaborSupplier laborSupplier_;
    GoodsDemander goodsDemander_;
    const AgentID id_;
    const int     operationDay_{instanceCnt++ % setting::dayInMonth};
};
}  // namespace abm