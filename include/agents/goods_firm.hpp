#pragma once

#include <algorithm>
#include "agents/common.hpp"
#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "world/labor.hpp"

namespace abm {
class GoodsFirm final : public Agent {
  public:
    explicit GoodsFirm(RandomGenerator& masterRng) noexcept
        : finance_{id_, masterRng},
          labor_{id_, masterRng, operationDay_},
          capital_{masterRng},
          goods_{masterRng} {}

    void act(const Date& date, MarketRegistry& markets) noexcept {
        if (date.day() == operationDay_ and date.isBeginingYear())
            actJanuaryOperatingDay(markets);
        else if (date.day() == operationDay_ and not date.isBeginingYear())
            actRegularOperatingDay(labor::toMarketPhase(date.month()), markets);
        else if (date.day() == operationDay_ - Day{1})
            actBeforeOperationDay();
        else if (date.day() == operationDay_ + Day{2})
            actAfterOperationDay();
        else
            actNothingDay();
    }

  private:
    void actJanuaryOperatingDay(MarketRegistry& markets) noexcept {
        const auto laborBudgetReq = [&]() -> Budget {
            const auto employee = labor_.employeeCnt();
            const auto adjust   = goods_.calcDesiredEmploy(employee);
            const auto sales    = goods_.salesForecast();
            return labor_.requestAnnualBudget(adjust, sales);
        }();
        const auto salesPlan        = goods_.planAndExpectSales(labor_.calcMonthlyCost());
        const auto capitalBudgetReq = capital_.planBudget(goods_.requiresCapital());

        const auto total = laborBudgetReq + capitalBudgetReq - salesPlan;
        if (total.isZeroOrLess()) {
            labor_.reviseAnnualPlan(laborBudgetReq);
            capital_.revisePlan(capitalBudgetReq);
        } else {
            const auto budget = finance_.claimBudget(total) + salesPlan;
            ASSERT(budget <= laborBudgetReq + capitalBudgetReq);

            const auto laborBudget   = std::min(budget, laborBudgetReq);
            const auto capitalBudget = std::max(budget - laborBudget, Budget{0.0});
            labor_.reviseAnnualPlan(laborBudget);
            capital_.revisePlan(capitalBudget);
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
        const auto laborCost        = labor_.calcMonthlyCost();
        const auto salesPlan        = goods_.planAndExpectSales(labor_.calcMonthlyCost());
        const auto capitalBudgetReq = capital_.planBudget(goods_.requiresCapital());

        const auto total = laborCost + capitalBudgetReq - salesPlan;
        if (total.isZeroOrLess()) {
            capital_.revisePlan(capitalBudgetReq);
        } else {
            const auto budget = finance_.claimBudget(total) + salesPlan;
            ASSERT(budget <= laborCost + capitalBudgetReq);
            capital_.revisePlan(std::min(budget - laborCost, Budget{0.0}));
        }

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
        goods_.endTrading();
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
};
}  // namespace abm