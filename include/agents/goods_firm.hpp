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
class GoodsFirm final {
    static inline constinit int instanceCnt{};

  public:
    explicit GoodsFirm(const AgentID id, RandomGenerator& masterRng) noexcept
        : finance_{id, masterRng},
          labor_{id, masterRng, Day{instanceCnt % setting::dayInMonth}},
          capital_{masterRng},
          goods_{masterRng},
          id_{id} {}

    void act(const Date& date, MarketRegistry& markets) noexcept {
        if (date.day() == operationDay_ and date.isBeginingYear())
            actJanuaryOperatingDay(markets);
        else if (date.day() == operationDay_ and not date.isBeginingYear())
            actRegularOperatingDay(labor::toMarketPhase(date.month()), markets);
        else if (date.day() == operationDay_ - Day{1})
            actBeforeOperationDay();
        else if (date.day() == operationDay_ + Day{1})
            actAfterOperationDay();
        else
            actNothingDay();
    }

  private:
    void actJanuaryOperatingDay(MarketRegistry& markets) noexcept {
        const auto employee       = labor_.employeeCnt();
        const auto adjustEmploy   = goods_.calcDesiredEmploy(employee);
        const auto sales          = goods_.salesForecast();
        const auto laborBudgetReq = labor_.requestAnnualBudget(adjustEmploy, sales);

        const auto totalCost = labor_.sumWage();
        const auto salesPlan = goods_.planAndExpectSales(totalCost);

        const auto desiredCapital   = goods_.requiresCapital();
        const auto capitalBudgetReq = capital_.planBudget(desiredCapital);

        const auto total = laborBudgetReq + capitalBudgetReq - salesPlan;
        if (total.isZeroOrLess()) {
            labor_.reviseAnnualPlan(laborBudgetReq);
            capital_.revisePlan(capitalBudgetReq);
            return;
        }

        const auto budget = finance_.claimBudget(total) + salesPlan;
        ASSERT(budget <= laborBudgetReq + capitalBudgetReq);

        if (budget < laborBudgetReq) {
            labor_.reviseAnnualPlan(budget);
            capital_.revisePlan(Budget{0.0});
        } else {
            labor_.reviseAnnualPlan(laborBudgetReq);
            capital_.revisePlan(budget - laborBudgetReq);
        }

        labor_.layOffs();
        labor_.postRequest(id_, markets.laborMarket);

        goods_.post(id_, markets.goodsMarket);

        capital_.request(
            id_,
            finance_.makeWithdrawFn(FirmFinance::AccountItem::CapitalGoodsCost),
            markets.capitalMarket
        );
    }

    void actRegularOperatingDay(const LaborMarketPhase phase, MarketRegistry& markets) noexcept {
        const auto laborCost = labor_.calcMonthlyCost();

        const auto totalCost = labor_.sumWage();
        const auto salesPlan = goods_.planAndExpectSales(totalCost);

        const auto desiredCapital   = goods_.requiresCapital();
        const auto capitalBudgetReq = capital_.planBudget(desiredCapital);

        const auto total  = laborCost + capitalBudgetReq - salesPlan;
        const auto budget = finance_.claimBudget(total) + salesPlan;
        ASSERT(budget <= laborCost + capitalBudgetReq);

        if (budget.isZeroOrLess())
            capital_.revisePlan(capitalBudgetReq);
        else if (budget < laborCost)
            capital_.revisePlan(Budget{0.0});
        else
            capital_.revisePlan(budget - laborCost);

        if (phase == LaborMarketPhase::Offer)
            labor_.offer();
        else if (phase == LaborMarketPhase::EndRecruiting)
            labor_.endRecruiting(goods_.workspace());

        capital_.request(
            id_,
            finance_.makeWithdrawFn(FirmFinance::AccountItem::CapitalGoodsCost),
            markets.capitalMarket
        );

        goods_.post(id_, markets.goodsMarket);
    }

    void actBeforeOperationDay() noexcept {
        labor_.payWage(finance_.makeWithdrawFn(FirmFinance::AccountItem::PersonalCost));
        goods_.trade(finance_.makeDepositFn(FirmFinance::AccountItem::Sales));
    }

    void actAfterOperationDay() noexcept {
        capital_.afterTrade(finance_.makeDepositFn(FirmFinance::AccountItem::CapitalGoodsCost));
        goods_.trade(finance_.makeDepositFn(FirmFinance::AccountItem::Sales));
    }

    void actNothingDay() noexcept {
        goods_.trade(finance_.makeDepositFn(FirmFinance::AccountItem::Sales));
    }

    FirmFinance     finance_;
    LaborDemander   labor_;
    CapitalDemander capital_;
    GoodsSupplier   goods_;
    const AgentID   id_;
    const Day       operationDay_{instanceCnt++ % setting::dayInMonth};
};
}  // namespace abm