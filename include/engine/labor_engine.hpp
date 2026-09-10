#pragma once

#include <span>

#include "engine/agents.hpp"
#include "system/labor.hpp"
#include "world/drop_box.hpp"
#include "world/labor.hpp"

namespace abm::engine {
class LaborEngine final {
  public:
    explicit LaborEngine(LaborDropBox& dropBox) noexcept : dropBox_{dropBox} {}

    void run(
        std::span<CapitalFirm> capitalFirms,
        std::span<GoodsFirm>   goodsFirms,
        std::span<HHold>       hholds
    ) noexcept {
        using namespace labor;
        for (auto& firm : capitalFirms) {
            adjustWorkforce(firm.id, firm.laborDemander, market_);
        }
        for (auto& firm : goodsFirms) {
            adjustWorkforce(firm.id, firm.laborDemander, market_);
        }

        for (auto& hhold : hholds) {
            entry(hhold.id, hhold.labor, market_);
        }

        for (auto& firm : capitalFirms) {
            offer(firm.laborDemander);
        }
        for (auto& firm : goodsFirms) {
            offer(firm.laborDemander);
        }

        for (auto& hhold : hholds) {
            accept(hhold.labor);
        }

        for (auto& firm : capitalFirms) {
            endRecruiting(firm.laborDemander, firm.capitalSupplier);
        }
        for (auto& firm : goodsFirms) {
            endRecruiting(firm.laborDemander, firm.goodsSupplier);
        }

        for (auto& hhold : hholds) {
            recordRosterEntry(hhold.labor);
        }

        for (auto& firm : capitalFirms) {
            logging(dropBox_, firm.laborDemander);
        }

        for (auto& firm : goodsFirms) {
            logging(dropBox_, firm.laborDemander);
        }

        for (auto& hhold : hholds) {
            logging(dropBox_, hhold.labor);
        }

        market_.clear();
    }

  private:
    LaborMarket   market_;
    LaborDropBox& dropBox_;
};
}  // namespace abm::engine