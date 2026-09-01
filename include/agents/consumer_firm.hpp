#pragma once

#include "agents/common.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/goods_supplier/goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"

namespace abm {
class GoodsFirm final {
    static inline constinit int instanceCnt{};

  public:
    explicit GoodsFirm(const AgentID id, RandomGenerator& masterRng) noexcept
        : finance_{id, masterRng},
          laborDemander_{id, masterRng},
          capitalDemander_{masterRng},
          goodsSupplier_{masterRng},
          id_{id} {}

    void act(const Date& today, Markets& markets) noexcept;

  private:
    FirmFinance     finance_;
    LaborDemander   laborDemander_;
    CapitalDemander capitalDemander_;
    GoodsSupplier   goodsSupplier_;
    const AgentID   id_;
    const int       businessDay_{instanceCnt++ % setting::dayInMonth};
};
}  // namespace abm