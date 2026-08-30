#pragma once

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class CapitalFirm final {
  public:
    [[nodiscard]] explicit CapitalFirm(RandomGenerator& masterRng) noexcept;

    void beginingYear() noexcept {
        const auto sales = capitalSupplier_.planAndExpectSales(laborDemander_.sumWage());
        const auto laborDemanderRequest   = laborDemanderAnnualRequestBudget();
        const auto capitalDemanderRequest = capitalDemanderRequestBudget();
        const auto total                  = laborDemanderRequest + capitalDemanderRequest - sales;
        if (total.isZeroOrLess()) {
            laborDemander_.reviseAnnualPlan(laborDemanderRequest);
            capitalDemander_.revisePlan(capitalDemanderRequest);
            return;
        }
        const auto budget = finance_.claimBudget(total) + sales;
        ASSERT(budget <= laborDemanderRequest + capitalDemanderRequest);
    }

  private:
    [[nodiscard]] auto laborDemanderAnnualRequestBudget() noexcept -> Money {
        const auto adjust = capitalSupplier_.calcDesiredEmploy(laborDemander_.employeeCnt());
        return laborDemander_.planAnnualAndRequestBudget(adjust, capitalSupplier_.salesForecast());
    }
    [[nodiscard]] auto capitalDemanderRequestBudget() noexcept -> Money {
        const auto desired = capitalSupplier_.requiresCapital();
        return capitalDemander_.planAndRequestBudget(desired);
    }
    void distributeAnnualBudget(
        const Money budget, const Money laborDemanderRequest, const Money capitalDemanderRequest
    ) noexcept {
        ASSERT(budget <= laborDemanderRequest + capitalDemanderRequest);
        if (budget < laborDemanderRequest) {
            laborDemander_.reviseAnnualPlan(budget);
            capitalDemander_.revisePlan(Money{0.0});
            return;
        }
        laborDemander_.reviseAnnualPlan(laborDemanderRequest);
        capitalDemander_.revisePlan(budget - laborDemanderRequest);
    }

    finance::Finance finance_;
    LaborDemander    laborDemander_;
    CapitalDemander  capitalDemander_;
    CapitalSupplier  capitalSupplier_;
    const AgentID    id_;
};
}  // namespace abm