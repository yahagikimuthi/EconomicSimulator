#pragma once

#include <algorithm>

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm::begining::detail {
[[nodiscard]] inline auto laborDemanderAnnualRequestBudget(
    LaborDemander& laborDemander, CapitalSupplier& capitalSupplier
) noexcept -> Money {
    const auto adjust = capitalSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    return laborDemander.planAnnualAndRequestBudget(adjust, capitalSupplier.salesForecast());
}

[[nodiscard]] inline auto capitalDemanderRequestBudget(
    CapitalDemander& capitalDemander, CapitalSupplier& capitalSupplier
) noexcept -> Money {
    const auto desired = capitalSupplier.requiresCapital();
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

[[nodiscard]] auto laborDemanderAnnualRequestBudget(
    LaborDemander& laborDemander, ConsumerGoodsSupplier& consumerGoodsSupplier
) noexcept -> Money {
    const auto adjust = consumerGoodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    return laborDemander.planAnnualAndRequestBudget(adjust, consumerGoodsSupplier.salesForecast());
}
[[nodiscard]] auto capitalDemanderRequestBudget(
    ConsumerGoodsSupplier& consumerGoodsSupplier, CapitalDemander& capitalDemander
) noexcept -> Money {
    const auto desired = consumerGoodsSupplier.requiresCapital();
    return capitalDemander.planAndRequestBudget(desired);
}
}  // namespace abm::begining::detail

namespace abm::begining {
inline void beginingYear(
    finance::Finance& finance,
    LaborDemander&    laborDemander,
    CapitalDemander&  capitalDemander,
    CapitalSupplier&  capitalSupplier
) noexcept {
    const auto salesForecast = capitalSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto laborDemanderRequest =
        detail::laborDemanderAnnualRequestBudget(laborDemander, capitalSupplier);
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(capitalDemander, capitalSupplier);
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
    finance::Finance& finance,
    LaborDemander&    laborDemander,
    CapitalDemander&  capitalDemander,
    CapitalSupplier&  capitalSupplier
) noexcept {
    const auto salesForecast = capitalSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto sumWage       = laborDemander.sumWage();
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(capitalDemander, capitalSupplier);
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
    finance::Finance&      finance,
    LaborDemander&         laborDemander,
    ConsumerGoodsSupplier& consumerGoodsSupplier,
    CapitalDemander&       capitalDemander
) noexcept {
    const auto salesForecast = consumerGoodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto laborDemanderRequest =
        detail::laborDemanderAnnualRequestBudget(laborDemander, consumerGoodsSupplier);
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(consumerGoodsSupplier, capitalDemander);
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
    finance::Finance&      finance,
    LaborDemander&         laborDemander,
    ConsumerGoodsSupplier& consumerGoodsSupplier,
    CapitalDemander&       capitalDemander
) noexcept {
    const auto salesForecast = consumerGoodsSupplier.planAndExpectSales(laborDemander.sumWage());
    const auto sumWage       = laborDemander.sumWage();
    const auto capitalDemanderRequest =
        detail::capitalDemanderRequestBudget(consumerGoodsSupplier, capitalDemander);
    const auto total = sumWage + capitalDemanderRequest - salesForecast;
    if (total.isZeroOrLess()) {
        capitalDemander.revisePlan(capitalDemanderRequest);
        return;
    }
    const auto budget = finance.claimBudget(total) + salesForecast;
    ASSERT(budget <= total);
    capitalDemander.revisePlan(std::max(budget - sumWage, Money{0.0}));
}
}  // namespace abm::begining