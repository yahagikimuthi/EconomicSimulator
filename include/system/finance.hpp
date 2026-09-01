#pragma once

#include <algorithm>

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm::finance::detail {
[[nodiscard]] inline auto laborDemanderAnnualRequestBudget(
    LaborDemander& laborDemander, BaseGoodsSupplier& goodsSupplier
) noexcept -> Budget {
    const auto adjust = goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    return laborDemander.planAnnualAndRequestBudget(adjust, goodsSupplier.salesForecast());
}

[[nodiscard]] inline auto capitalDemanderRequestBudget(
    CapitalDemander& capitalDemander, BaseGoodsSupplier& goodsSupplier
) noexcept -> Budget {
    const auto desired = goodsSupplier.requiresCapital();
    return capitalDemander.planAndRequestBudget(desired);
}

inline void distributeAnnualBudget(
    LaborDemander&   laborDemander,
    CapitalDemander& capitalDemander,
    const Budget     budget,
    const Budget     laborDemanderRequest,
    const Budget     capitalDemanderRequest
) noexcept {
    ASSERT(budget <= laborDemanderRequest + capitalDemanderRequest);
    if (budget < laborDemanderRequest) {
        laborDemander.reviseAnnualPlan(budget);
        capitalDemander.revisePlan(Budget{0.0});
        return;
    }
    laborDemander.reviseAnnualPlan(laborDemanderRequest);
    capitalDemander.revisePlan(laborDemanderRequest - budget);
}

[[nodiscard]] auto capitalDemanderRequestBudget(
    BaseGoodsSupplier& goodsSupplier, CapitalDemander& capitalDemander
) noexcept -> Budget {
    const auto desired = goodsSupplier.requiresCapital();
    return capitalDemander.planAndRequestBudget(desired);
}
}  // namespace abm::finance::detail

namespace abm::finance {
inline void judgeYearlyAndMonthlyBudget(
    FirmFinance&       finance,
    LaborDemander&     laborDemander,
    CapitalDemander&   capitalDemander,
    BaseGoodsSupplier& goodsSupplier
) noexcept {
    const auto salesForecast = goodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto laborDemanderRequest =
        detail::laborDemanderAnnualRequestBudget(laborDemander, goodsSupplier);
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(capitalDemander, goodsSupplier);
    const auto total = laborDemanderRequest + capitalDemanderRequest - salesForecast;
    if (total.isZeroOrLess()) {
        laborDemander.reviseAnnualPlan(laborDemanderRequest);
        capitalDemander.revisePlan(capitalDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + salesForecast;
    ASSERT(budget <= laborDemanderRequest + capitalDemanderRequest);
    detail::distributeAnnualBudget(
        laborDemander, capitalDemander, budget, laborDemanderRequest, capitalDemanderRequest
    );
}

inline void judgeMonthlyBudget(
    FirmFinance&       finance,
    LaborDemander&     laborDemander,
    CapitalDemander&   capitalDemander,
    BaseGoodsSupplier& goodsSupplier
) noexcept {
    const auto salesForecast = goodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto sumWage       = static_cast<Budget>(laborDemander.sumWage());
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(capitalDemander, goodsSupplier);
    const auto total = static_cast<Budget>(sumWage) + capitalDemanderRequest - salesForecast;
    if (total.isZeroOrLess()) {
        capitalDemander.revisePlan(capitalDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + salesForecast;
    ASSERT(budget <= total);
    capitalDemander.revisePlan(std::max(budget - sumWage, Budget{0.0}));
}

inline void judgeYearlyAndMonthlyBudget(
    FirmFinance&       finance,
    LaborDemander&     laborDemander,
    BaseGoodsSupplier& goodsSupplier,
    CapitalDemander&   capitalDemander
) noexcept {
    const auto salesForecast = goodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto laborDemanderRequest =
        detail::laborDemanderAnnualRequestBudget(laborDemander, goodsSupplier);
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(goodsSupplier, capitalDemander);
    const auto total = laborDemanderRequest + capitalDemanderRequest - salesForecast;
    if (total.isZeroOrLess()) {
        laborDemander.reviseAnnualPlan(laborDemanderRequest);
        capitalDemander.revisePlan(capitalDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + salesForecast;
    ASSERT(budget <= total);
    detail::distributeAnnualBudget(
        laborDemander, capitalDemander, budget, laborDemanderRequest, capitalDemanderRequest
    );
}

inline void judgeMonthlyBudget(
    FirmFinance&       finance,
    LaborDemander&     laborDemander,
    BaseGoodsSupplier& goodsSupplier,
    CapitalDemander&   capitalDemander
) noexcept {
    const auto salesForecast = goodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto sumWage       = static_cast<Budget>(laborDemander.sumWage());
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(goodsSupplier, capitalDemander);
    const auto total = sumWage + capitalDemanderRequest - salesForecast;
    if (total.isZeroOrLess()) {
        capitalDemander.revisePlan(capitalDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + salesForecast;
    ASSERT(budget <= total);
    capitalDemander.revisePlan(std::max(budget - sumWage, Budget{0.0}));
}

inline void judgeMonthlyBudget(
    HHoldFinance& finance, LaborSupplier& laborSupplier, GoodsDemander& goodsDemander
) noexcept {
    const auto wage = static_cast<Budget>(laborSupplier.wage());
    const auto goodsDemanderRequest =
        goodsDemander.planAndRequestBudget(static_cast<Budget>(finance.asset()) + wage);
    const auto total = goodsDemanderRequest - wage;
    if (total.isZeroOrLess()) {
        goodsDemander.revisePlan(goodsDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + wage;
    ASSERT(budget <= goodsDemanderRequest);
    goodsDemander.revisePlan(budget);
}
}  // namespace abm::finance