#pragma once

#include <algorithm>
#include "components/capital_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/labor.hpp"

namespace abm {
struct ConsumerFirm final {
    finance::Finance      finance;
    LaborDemander         laborDemander;
    CapitalDemander       capitalDemander;
    ConsumerGoodsSupplier consumerGoodsSupplier;
    const AgentID         id;
};
}  // namespace abm

namespace abm::consumer_firm::detail {
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

void distributeAnnualBudget(
    LaborDemander&   laborDemander,
    CapitalDemander& capitalDemander,
    const Money      budget,
    const Money      laborDemanderDesired,
    const Money      capitalDemanderDesired
) noexcept {
    ASSERT(budget <= laborDemanderDesired + capitalDemanderDesired);
    if (budget < laborDemanderDesired) {
        laborDemander.reviseAnnualPlan(budget);
        capitalDemander.revisePlan(Money{0.0});
        return;
    }
    laborDemander.reviseAnnualPlan(laborDemanderDesired);
    capitalDemander.revisePlan(budget - laborDemanderDesired);
}
}  // namespace abm::consumer_firm::detail

namespace abm::consumer_firm {
void beginingYear(
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

void beginingMonth(
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

void layOffs(LaborDemander& laborDemander) noexcept { laborDemander.layOffs(); }

void postLaborRequest(
    const AgentID id, LaborDemander& laborDemander, LaborMarket& market
) noexcept {
    laborDemander.postLaborRequest(id, market);
}

void offer(LaborDemander& laborDemander) noexcept { laborDemander.offer(); }

void registerMember(LaborDemander& laborDemander, ConsumerGoodsSupplier& goodsSupplier) noexcept {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void acceptResignation(LaborDemander& laborDemander) noexcept { laborDemander.acceptResignation(); }
}  // namespace abm::consumer_firm