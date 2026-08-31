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

namespace abm::begining::detail {
[[nodiscard]] inline auto laborDemanderAnnualRequestBudget(
    LaborDemander& laborDemander, BaseGoodsSupplier& goodsSupplier
) noexcept -> Money {
    const auto adjust = goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    return laborDemander.planAnnualAndRequestBudget(adjust, goodsSupplier.salesForecast());
}

[[nodiscard]] inline auto capitalDemanderRequestBudget(
    CapitalDemander& capitalDemander, BaseGoodsSupplier& goodsSupplier
) noexcept -> Money {
    const auto desired = goodsSupplier.requiresCapital();
    return capitalDemander.planAndRequestBudget(desired);
}

inline void distributeAnnualBudget(
    LaborDemander&   laborDemander,
    CapitalDemander& capitalDemander,
    const Money      budget,
    const Money      laborDemanderRequest,
    const Money      capitalDemanderRequest
) noexcept {
    ASSERT(budget <= laborDemanderRequest + capitalDemanderRequest);
    if (budget < laborDemanderRequest) {
        laborDemander.reviseAnnualPlan(budget);
        capitalDemander.revisePlan(Money{0.0});
        return;
    }
    laborDemander.reviseAnnualPlan(laborDemanderRequest);
    capitalDemander.revisePlan(laborDemanderRequest - budget);
}

[[nodiscard]] auto capitalDemanderRequestBudget(
    BaseGoodsSupplier& goodsSupplier, CapitalDemander& capitalDemander
) noexcept -> Money {
    const auto desired = goodsSupplier.requiresCapital();
    return capitalDemander.planAndRequestBudget(desired);
}
}  // namespace abm::begining::detail

namespace abm::begining {
inline void beginingYear(
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

inline void beginingMonth(
    FirmFinance&       finance,
    LaborDemander&     laborDemander,
    CapitalDemander&   capitalDemander,
    BaseGoodsSupplier& goodsSupplier
) noexcept {
    const auto salesForecast = goodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto sumWage       = laborDemander.sumWage();
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(capitalDemander, goodsSupplier);
    const auto total = sumWage + capitalDemanderRequest - salesForecast;
    if (total.isZeroOrLess()) {
        capitalDemander.revisePlan(capitalDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + salesForecast;
    ASSERT(budget <= total);
    capitalDemander.revisePlan(std::max(budget - sumWage, Money{0.0}));
}

inline void beginingYear(
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

inline void beginingMonth(
    FirmFinance&       finance,
    LaborDemander&     laborDemander,
    BaseGoodsSupplier& goodsSupplier,
    CapitalDemander&   capitalDemander
) noexcept {
    const auto salesForecast = goodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto sumWage       = laborDemander.sumWage();
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(goodsSupplier, capitalDemander);
    const auto total = sumWage + capitalDemanderRequest - salesForecast;
    if (total.isZeroOrLess()) {
        capitalDemander.revisePlan(capitalDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + salesForecast;
    ASSERT(budget <= total);
    capitalDemander.revisePlan(std::max(budget - sumWage, Money{0.0}));
}

inline void beginingMonth(
    HHoldFinance& finance, LaborSupplier& laborSupplier, GoodsDemander& goodsDemander
) noexcept {
    const auto goodsDemanderRequest =
}
}  // namespace abm::begining