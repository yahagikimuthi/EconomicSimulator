#pragma once

#include "agents/common.hpp"
#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "world/labor.hpp"

namespace abm {
class CapitalFirm final {
    static inline constinit int instanceCnt{};

  public:
    CapitalFirm(const AgentID id, RandomGenerator& masterRng) noexcept
        : finance_{id, masterRng},
          labor_{id, masterRng, Day{(instanceCnt % (setting::dayInMonth - 1)) + 2}},
          capitalDemander_{masterRng},
          capitalSupplier_{masterRng},
          id_{id} {}

    void act(const Date& date, MarketRegistry& markets) noexcept {
        if (date.day() == operationDay_) {
            if (date.month() == Month{1})
                actJanuaryOperationDay(markets);
            else
                actRegularOperationDay(labor::toMarketPhase(date.month()), markets);
        } else if (date.day() == operationDay_ - Day{1}) {
            actBeforeOperationDay();
        } else if (date.day() == operationDay_ + Day{1}) {
            actAfterOperationDay();
        } else
            actNothingDay();
    }

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
            ASSERT(budget <= laborReqBudget + capitalReqBudget);

            if (budget < laborReqBudget) {
                labor_.reviseAnnualPlan(budget);
                capitalDemander_.revisePlan(Budget{0.0});
            } else {
                labor_.reviseAnnualPlan(laborReqBudget);
                capitalDemander_.revisePlan(budget - laborReqBudget);
            }
        }

        labor_.layOffs();
        labor_.postRequest(id_, markets.laborMarket);

        capitalDemander_.request(
            id_,
            finance_.makeWithdrawFn(FirmFinance::AccountItem::CapitalGoodsCost),
            markets.capitalMarket
        );

        capitalSupplier_.post(id_, markets.capitalMarket);
    }

    void actRegularOperationDay(const LaborMarketPhase phase, MarketRegistry& markets) noexcept {
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

        if (phase == LaborMarketPhase::Offer)
            labor_.offer();
        else if (phase == LaborMarketPhase::EndRecruiting)
            labor_.endRecruiting(capitalSupplier_.workspace());

        capitalDemander_.request(
            id_,
            finance_.makeWithdrawFn(FirmFinance::AccountItem::CapitalGoodsCost),
            markets.capitalMarket
        );

        capitalSupplier_.post(id_, markets.capitalMarket);
    }

    void actBeforeOperationDay() noexcept {
        labor_.payWage(finance_.makeWithdrawFn(FirmFinance::AccountItem::PersonalCost));
        capitalSupplier_.trade(finance_.makeDepositFn(FirmFinance::AccountItem::Sales));
        capitalSupplier_.endTrading();
    }

    void actAfterOperationDay() noexcept {
        capitalDemander_.afterTrade(
            finance_.makeDepositFn(FirmFinance::AccountItem::CapitalGoodsCost)
        );
        capitalSupplier_.trade(finance_.makeDepositFn(FirmFinance::AccountItem::Sales));
    }

    void actNothingDay() noexcept {
        capitalSupplier_.trade(finance_.makeDepositFn(FirmFinance::AccountItem::Sales));
    }

    FirmFinance     finance_;
    LaborDemander   labor_;
    CapitalDemander capitalDemander_;
    CapitalSupplier capitalSupplier_;
    const AgentID   id_;
    const Day       operationDay_{(instanceCnt++ % (setting::dayInMonth - 1)) + 2};
};
}  // namespace abm