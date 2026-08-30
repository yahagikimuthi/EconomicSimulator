#pragma once

#include "components/capital_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class ConsumerFirm {
  public:
    [[nodiscard]] ConsumerFirm(RandomGenerator& rng) noexcept;

    void beginingYear() noexcept {
        const auto salesForecast =
            consumerGoodsSupplier_.planAndExpectSales(laborDemander_.sumWage());
        const auto laborDemanderRequest   = calcLaborDemanderRequestBudget(salesForecast);
        const auto capitalDemanderRequest = calcCapitalDemanderRequestBudget();
        const auto total = laborDemanderRequest + capitalDemanderRequest - salesForecast;
        if (total.isZeroOrLess()) {
            laborDemander_.reviseAnnualPlan(laborDemanderRequest);
            capitalDemander_.revisePlan(capitalDemanderRequest);
            return;
        }
        const auto budget = finance_.claimBudget(total);
        ASSERT(budget <= total);
        distributeBudget(budget, laborDemanderRequest, capitalDemanderRequest);
    }

  private:
    [[nodiscard]] auto calcLaborDemanderRequestBudget(const Money salesForecast) noexcept -> Money {
        const auto adjust = consumerGoodsSupplier_.calcDesiredEmploy(laborDemander_.employeeCnt());
        return laborDemander_.planAnnualAndRequestBudget(adjust, salesForecast);
    }
    [[nodiscard]] auto calcCapitalDemanderRequestBudget() noexcept -> Money {
        const auto desired = consumerGoodsSupplier_.requiresCapital();
        return capitalDemander_.planAndRequestBudget(desired);
    }

    void distributeBudget(
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

    finance::Finance      finance_;
    LaborDemander         laborDemander_;
    CapitalDemander       capitalDemander_;
    ConsumerGoodsSupplier consumerGoodsSupplier_;
    const AgentID         id_;
};
}  // namespace abm