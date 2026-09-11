#pragma once

#include <algorithm>
#include <cassert>

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "values/common.hpp"

namespace abm::planning {
inline void planJanuary(
    FirmFinance& finance, LaborDemander& labor, CapitalDemander& capital, GoodsSupplier& goods
) noexcept {
    const auto laborReq = [&]() noexcept -> Budget {
        const auto employee = labor.employeeCnt();
        const auto adjust   = goods.desiredEmploy(employee);
        const auto sales    = goods.salesForecast();
        return labor.requestAnnualBudget(adjust, sales);
    }();
    const auto salesPlan  = goods.planAndExpectSales(labor.calcMonthlyCost());
    const auto capitalReq = capital.planBudget(goods.desiredCapital());

    const auto total = laborReq + capitalReq - salesPlan;
    if (total.isZeroOrLess()) {
        labor.reviseAnnualPlan(laborReq);
        capital.revisePlan(capitalReq);
    } else {
        const auto budget = finance.claimBudget(total) + salesPlan;
        assert(budget <= laborReq + capitalReq);

        const auto laborBudget   = std::min(budget, laborReq);
        const auto capitalBudget = std::max(budget - laborBudget, Budget{0.0});

        labor.reviseAnnualPlan(laborBudget);
        capital.revisePlan(capitalBudget);
    }
}

inline void planJanuary(
    FirmFinance&     finance,
    LaborDemander&   labor,
    CapitalDemander& capitalDemander,
    CapitalSupplier& capitalSupplier
) noexcept {
    const auto laborReq = [&]() noexcept -> Budget {
        const auto employee = labor.employeeCnt();
        const auto adjust   = capitalSupplier.desiredEmploy(employee);
        const auto sales    = capitalSupplier.salesForecast();
        return labor.requestAnnualBudget(adjust, sales);
    }();
    const auto salesPlan  = capitalSupplier.planAndExpectSales(labor.calcMonthlyCost());
    const auto capitalReq = capitalDemander.planBudget(capitalSupplier.desiredCapital());

    const auto total = laborReq + capitalReq - salesPlan;
    if (total.isZeroOrLess()) {
        labor.reviseAnnualPlan(laborReq);
        capitalDemander.revisePlan(capitalReq);
    } else {
        const auto budget = finance.claimBudget(total) + salesPlan;
        assert(budget <= laborReq + capitalReq);

        const auto laborBudget   = std::min(budget, laborReq);
        const auto capitalBudget = std::max(budget - laborBudget, Budget{0.0});

        labor.reviseAnnualPlan(laborBudget);
        capitalDemander.revisePlan(capitalBudget);
    }
}

inline void planJanuary(HHoldFinance& finance, GoodsDemander& goods) noexcept {
    const auto purchasePlan = goods.requestBudget(finance.asset());
    assert(purchasePlan.isZeroOrMore());
    const auto budget = goods.requestBudget(purchasePlan);
    goods.revisePlan(budget);
}

inline void planStandard(
    FirmFinance&     finance,
    LaborDemander&   labor,
    CapitalDemander& capitalDemander,
    CapitalSupplier& capitalSupplier
) noexcept {
    const auto laborCost  = labor.calcMonthlyCost();
    const auto salesPlan  = capitalSupplier.planAndExpectSales(labor.calcMonthlyCost());
    const auto capitalReq = capitalDemander.planBudget(capitalSupplier.desiredCapital());

    const auto total = laborCost + capitalReq - salesPlan;
    if (total.isZeroOrLess()) {
        capitalDemander.revisePlan(capitalReq);
    } else {
        const auto budget = finance.claimBudget(total) + salesPlan;
        assert(budget <= laborCost + capitalReq);

        const auto capitalBudget = std::max(budget - laborCost, Budget{0.0});
        capitalDemander.revisePlan(capitalBudget);
    }
}

inline void planStandard(
    FirmFinance& finance, LaborDemander& labor, CapitalDemander& capital, GoodsSupplier& goods
) noexcept {
    const auto laborCost  = labor.calcMonthlyCost();
    const auto salesPlan  = goods.planAndExpectSales(labor.calcMonthlyCost());
    const auto capitalReq = capital.planBudget(goods.desiredCapital());

    const auto total = laborCost + capitalReq - salesPlan;
    if (total.isZeroOrLess()) {
        capital.revisePlan(capitalReq);
    } else {
        const auto budget = finance.claimBudget(total) + salesPlan;
        assert(budget <= laborCost + capitalReq);

        const auto capitalBudget = std::max(budget - laborCost, Budget{0.0});
        capital.revisePlan(capitalBudget);
    }
}

inline void planStandard(HHoldFinance& finance, GoodsDemander& goods) noexcept {
    planJanuary(finance, goods);
}
}  // namespace abm::planning