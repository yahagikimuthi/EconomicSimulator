#pragma once

#include <span>

#include "components/government.hpp"
#include "engine/agents.hpp"
#include "system/end_month.hpp"
#include "world/drop_box.hpp"

namespace abm::engine {
class EndMonthEngine final {
  public:
    explicit EndMonthEngine(FinanceDropBox& dropBox) noexcept : dropBox_{dropBox} {}

    void run(
        std::span<CapitalFirm> capitalFirms,
        std::span<GoodsFirm>   goodsFirms,
        std::span<HHold>       hholds,
        Government&            gov,
        CensusDropBox&         dropBox
    ) noexcept {
        using namespace end_month;

        for (auto& firm : capitalFirms) {
            payWage(firm.finance, firm.laborDemander, gov);
        }
        for (auto& firm : goodsFirms) {
            payWage(firm.finance, firm.laborDemander, gov);
        }

        for (auto& hhold : hholds) {
            workAndReceiveWage(hhold.finance, hhold.labor);
        }

        for (auto& firm : capitalFirms) {
            logging(dropBox_, firm.finance);
        }
        for (auto& firm : goodsFirms) {
            logging(dropBox_, firm.finance);
        }

        for (auto& hhold : hholds) {
            logging(dropBox_, hhold.finance);
        }

        gov.setBudget(dropBox);

        for (auto& firm : capitalFirms) {
            finalizeAccounts(firm.finance, gov);
        }
        for (auto& firm : goodsFirms) {
            finalizeAccounts(firm.finance, gov);
        }

        for (auto& hhold : hholds) {
            provideUnemploymentBenefit(hhold.finance, hhold.labor, gov);
        }

        gov.reset();
    }

  private:
    FinanceDropBox& dropBox_;
};
}  // namespace abm::engine