#pragma once

#include <algorithm>

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/capital.hpp"

namespace abm {
struct CapitalFirm final {
    finance::Finance finance;
    LaborDemander    laborDemander;
    CapitalDemander  capitalDemander;
    CapitalSupplier  capitalSupplier;
    const AgentID    id;
};
}  // namespace abm

namespace abm::capital_firm::detail {
[[nodiscard]] auto laborDemanderAnnualRequestBudget(
    LaborDemander& laborDemander, CapitalSupplier& capitalSupplier
) noexcept -> Money {
    const auto adjust = capitalSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    return laborDemander.planAnnualAndRequestBudget(adjust, capitalSupplier.salesForecast());
}

[[nodiscard]] auto capitalDemanderRequestBudget(
    CapitalDemander& capitalDemander, CapitalSupplier& capitalSupplier
) noexcept -> Money {
    const auto desired = capitalSupplier.requiresCapital();
    return capitalDemander.planAndRequestBudget(desired);
}

void distributeAnnualBudget(
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
}  // namespace abm::capital_firm::detail

namespace abm::capital_firm {
void beginingYear(
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

void beginingMonth(
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

void layOffs(LaborDemander& laborDemander) noexcept { laborDemander.layOffs(); }

void postLaborRequest(
    const AgentID id, LaborDemander& laborDemander, LaborMarket& market
) noexcept {
    laborDemander.postLaborRequest(id, market);
}

void offer(LaborDemander& laborDemander) noexcept { laborDemander.offer(); }

void registerMember(LaborDemander& laborDemander, CapitalSupplier& goodsSupplier) noexcept {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void acceptResignation(LaborDemander& laborDemander) noexcept { laborDemander.acceptResignation(); }

void supplyCapital(const AgentID id, CapitalSupplier& supplier, CapitalMarket& market) noexcept {
    supplier.post(id, market);
}

void tradeCapital(CapitalSupplier& supplier) noexcept { supplier.trade(); }

void purchaseCapital(const AgentID id, CapitalDemander& demander, CapitalMarket& market) noexcept {
    demander.request(id, market);
}

void afterCapitalTrade(CapitalDemander& demander) noexcept { demander.afterTrade(); }

}  // namespace abm::capital_firm