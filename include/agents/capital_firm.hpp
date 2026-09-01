#pragma once

#include "agents/common.hpp"
#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"

namespace abm {
class CapitalFirm final {
    static inline constinit int instanceCnt{};

  public:
    CapitalFirm(const AgentID id, RandomGenerator& masterRng) noexcept
        : finance_{id, masterRng},
          labor_{id, masterRng},
          capitalDemander_{masterRng},
          capitalSupplier_{masterRng},
          id_{id} {}

    void act(const Date& today, MarketRegistry& markets) noexcept;

  private:
    void actJanuaryOperationDay(MarketRegistry& markets) noexcept {
        const auto employee       = labor_.employeeCnt();
        const auto adjust         = capitalSupplier_.calcDesiredEmploy(employee);
        const auto sales          = capitalSupplier_.salesForecast();
        const auto laborReqBudget = labor_.requestAnnualBudget(adjust, sales);

        const auto desiredCapital   = capitalSupplier_.requiresCapital();
        const auto capitalReqBudget = capitalDemander_.planBudget(desiredCapital);

        const auto totalCost = labor_.sumWage();
        const auto salesPlan = capitalSupplier_.planAndExpectSales(totalCost);

        const auto total = laborReqBudget + capitalReqBudget - salesPlan;
        if (total.isZeroOrLess()) {
            labor_.reviseAnnualPlan(laborReqBudget);
            capitalDemander_.revisePlan(capitalReqBudget);
        } else {
            const auto budget = finance_.claimBudget(total) + salesPlan;
            ASSERT(budget < laborReqBudget + capitalReqBudget);

            if (budget < laborReqBudget) {
                labor_.reviseAnnualPlan(budget);
                capitalDemander_.revisePlan(Budget{0.0});
            } else {
                labor_.reviseAnnualPlan(laborReqBudget);
                capitalDemander_.revisePlan(budget - laborReqBudget);
            }
        }

        labor_.layOffs();
        labor_.postLaborRequest(id_, markets.laborMarket);

        capitalDemander_.request(id_, finance_, markets.capitalMarket);

        capitalSupplier_.post(id_, markets.capitalMarket);
    }

    void actRegularOperationDay(const int month, MarketRegistry& markets) noexcept {
        const auto laborCost = labor_.calcMonthlyCost();

        const auto desiredCapital   = capitalSupplier_.requiresCapital();
        const auto capitalReqBudget = capitalDemander_.planBudget(desiredCapital);

        const auto totalCost = labor_.sumWage();
        const auto salesPlan = capitalSupplier_.planAndExpectSales(totalCost);

        const auto total = laborCost + capitalReqBudget - salesPlan;
        if (total.isZeroOrLess()) {
            capitalDemander_.revisePlan(capitalReqBudget);
        } else {
            const auto budget = finance_.claimBudget(total);
            if (budget < laborCost) {
                capitalDemander_.revisePlan(Budget{0.0});
            }
        }

        ASSERT(month > 1);
        if (month == 3)
            labor_.offer();
        else if (month == 5)
            labor_.endRecruiting(capitalSupplier_.workspace());

        capitalDemander_.request(id_, finance_, markets.capitalMarket);

        capitalSupplier_.post(id_, markets.capitalMarket);
    }

    void actBeforeOperationDay() noexcept;

    void actAfterOperationDay() noexcept { capitalDemander_.afterTrade(finance_); }

    void actNothingDay() noexcept;

    FirmFinance     finance_;
    LaborDemander   labor_;
    CapitalDemander capitalDemander_;
    CapitalSupplier capitalSupplier_;
    const AgentID   id_;
    const int       operationDay_{instanceCnt++ % setting::dayInMonth};
};
}  // namespace abm