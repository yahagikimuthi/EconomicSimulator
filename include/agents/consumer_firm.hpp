#pragma once

#include "components/capital_goods_demander.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "others/util.hpp"
#include "world/capital_goods.hpp"
#include "world/labor.hpp"

namespace abm {
class ConsumerFirm {
  public:
    [[nodiscard]] ConsumerFirm(RandomGenerator& rng) noexcept;

    void beginingYear() noexcept {
        const auto laborDemanderDesiredBudget   = calcLaborDemanderDesiredBudget();
        const auto capitalGoodsDemanderDesired  = calcCapitalGoodsDemanderDesiredBudget();
        const auto consumerGoodsSupplierDesired = calcGoodsDemanderDesiredBudget();
        const auto totalRequiresMoney =
            laborDemanderDesiredBudget + capitalGoodsDemanderDesired + consumerGoodsSupplierDesired;
        if (totalRequiresMoney.isZeroOrLess()) {
            laborDemander_.revisePlan(laborDemanderDesiredBudget);
            capitalGoodsDemander_.revisePlan(capitalGoodsDemanderDesired);
            consumerGoodsSupplier_.revisePlan(consumerGoodsSupplierDesired);
            return;
        }
        const auto budget = finance_.claimBudget(totalRequiresMoney);
        if (budget < laborDemanderDesiredBudget) {
            laborDemander_.revisePlan(budget);
            capitalGoodsDemander_.revisePlan(Money{0.0});
            return;
        }
        laborDemander_.revisePlan(laborDemanderDesiredBudget);
        capitalGoodsDemander_.revisePlan(budget - laborDemanderDesiredBudget);
    }

    void beginingMonth() noexcept {
        consumerGoodsSupplier_.beginingMonth(laborDemander_.sumWage());
        const auto desiredCapitalGoods = consumerGoodsSupplier_.requiresCapitalGoods();
        capitalGoodsDemander_.plan(desiredCapitalGoods);
    }
    void endingMonth() noexcept;

    void adjustWorkforce(LaborMarket& market) noexcept {
        laborDemander_.adjustWorkforce(id_, market);
    }
    void offer() noexcept { laborDemander_.offer(); }
    void registerMember() noexcept {
        laborDemander_.registerMember(consumerGoodsSupplier_.workspace());
    }
    void acceptResignation() noexcept { laborDemander_.acceptResignation(); }

    void requestCapitalGoods(CapitalGoodsMarket& market) noexcept {
        capitalGoodsDemander_.request(id_, market);
    }
    void afterCapitalGoodsTrade() noexcept { capitalGoodsDemander_.afterTrade(); }

    void entryConsumerGoods(ConsumerGoodsMarket& market) noexcept {
        consumerGoodsSupplier_.post(market);
    }

  private:
    [[nodiscard]] auto calcLaborDemanderDesiredBudget() noexcept -> Money {
        const auto adjust = consumerGoodsSupplier_.calcDesiredEmploy(laborDemander_.employeeCnt());
        return laborDemander_.planAndRequestBudget(adjust, consumerGoodsSupplier_.salesForecast());
    }
    [[nodiscard]] auto calcCapitalGoodsDemanderDesiredBudget() noexcept -> Money {
        const auto desiredGoods = consumerGoodsSupplier_.requiresCapitalGoods();
        return capitalGoodsDemander_.planAndRequestBudget(desiredGoods);
    }
    [[nodiscard]] auto calcGoodsDemanderDesiredBudget() noexcept -> Money {
        const auto wage = laborDemander_.sumWage();
        return consumerGoodsSupplier_.planAndRequestBudget(wage);
    }

    finance::Finance      finance_;
    LaborDemander         laborDemander_;
    CapitalGoodsDemander  capitalGoodsDemander_;
    ConsumerGoodsSupplier consumerGoodsSupplier_;
    const AgentID         id_;
};
}  // namespace abm