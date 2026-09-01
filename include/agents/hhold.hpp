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
        : finance_{id, masterRng}, labor_{masterRng}, goods_{masterRng}, id_{id} {}

    void act(const Date& date, MarketRegistry& markets) noexcept {
        labor_.product();
        const auto day = date.day();
        if (day == operationDay_)
            actOperationDay(date.month(), markets);
        else if (day == operationDay_ + 1)
            actAfterOperationDay();
    }

  private:
    void actOperationDay(const int month, MarketRegistry& markets) noexcept {
        const auto wage         = static_cast<Budget>(labor_.wage());
        const auto asset        = static_cast<Budget>(finance_.asset());
        const auto purchasePlan = goods_.planAndRequestBudget(asset + wage);
        const auto total        = purchasePlan - wage;
        if (total.isZeroOrLess()) {
            goods_.revisePlan(purchasePlan);
            return;
        }
        const auto budget = finance_.claimBudget(total) + wage;
        ASSERT(budget <= total + wage);
        goods_.revisePlan(budget);

        if (month == 2)
            labor_.entry(id_, markets.laborMarket);
        else if (month == 4)
            labor_.accept();
        else if (month == 5)
            labor_.recordRosterEntry();

        goods_.request(id_, finance_, markets.goodsMarket);
    }

    void actAfterOperationDay() noexcept { goods_.afterTrade(finance_); }

    HHoldFinance  finance_;
    LaborSupplier labor_;
    GoodsDemander goods_;
    const AgentID id_;
    const int     operationDay_{instanceCnt++ % setting::dayInMonth};
};
}  // namespace abm