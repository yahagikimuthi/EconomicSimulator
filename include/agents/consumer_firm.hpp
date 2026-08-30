#pragma once

#include <algorithm>
#include "components/capital_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm {
class ConsumerFirm final {
  public:
    [[nodiscard]] ConsumerFirm(RandomGenerator& rng) noexcept;

    void beginingYear() noexcept {
        const auto salesForecast =
            consumerGoodsSupplier_.planAndExpectSales(laborDemander_.sumWage());
        const auto laborDemanderRequest   = laborDemanderAnnualRequestBudget();
        const auto capitalDemanderRequest = capitalDemanderRequestBudget();
        const auto total = laborDemanderRequest + capitalDemanderRequest - salesForecast;
        if (total.isZeroOrLess()) {
            laborDemander_.reviseAnnualPlan(laborDemanderRequest);
            capitalDemander_.revisePlan(capitalDemanderRequest);
            return;
        }
        const auto budget = finance_.claimBudget(total) + salesForecast;
        ASSERT(budget <= total);
        distributeAnnualBudget(budget, laborDemanderRequest, capitalDemanderRequest);
    }

    void beginintMonth() noexcept {
        const auto salesForecast =
            consumerGoodsSupplier_.planAndExpectSales(laborDemander_.sumWage());
        const auto sumWage                = laborDemander_.sumWage();
        const auto capitalDemanderRequest = capitalDemanderRequestBudget();
        const auto total                  = sumWage + capitalDemanderRequest - salesForecast;
        if (total.isZeroOrLess()) {
            capitalDemander_.revisePlan(capitalDemanderRequest);
            return;
        }
        const auto budget = finance_.claimBudget(total) + salesForecast;
        ASSERT(budget <= total);
        capitalDemander_.revisePlan(std::max(budget - sumWage, Money{0.0}));
    }

  private:
    [[nodiscard]] auto laborDemanderAnnualRequestBudget() noexcept -> Money {
        const auto adjust = consumerGoodsSupplier_.calcDesiredEmploy(laborDemander_.employeeCnt());
        return laborDemander_.planAnnualAndRequestBudget(
            adjust, consumerGoodsSupplier_.salesForecast()
        );
    }
    [[nodiscard]] auto capitalDemanderRequestBudget() noexcept -> Money {
        const auto desired = consumerGoodsSupplier_.requiresCapital();
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

    finance::Finance      finance_;
    LaborDemander         laborDemander_;
    CapitalDemander       capitalDemander_;
    ConsumerGoodsSupplier consumerGoodsSupplier_;
    const AgentID         id_;
};
}  // namespace abm