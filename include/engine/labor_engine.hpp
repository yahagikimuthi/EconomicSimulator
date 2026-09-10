#pragma once

#include <span>

#include "engine/agents.hpp"
#include "system/labor.hpp"
#include "world/drop_box.hpp"
#include "world/labor.hpp"

namespace abm::engine {
class LaborEngine {
  public:
    explicit LaborEngine() noexcept = default;

    void run(
        std::span<CapitalFirm> capitalFirm,
        std::span<GoodsFirm>   goodsFirm,
        std::span<HHold>       hholds,
        LaborDropBox&          dropBox
    ) noexcept {
        using namespace labor;
        for (auto& firm : capitalFirm) {
            adjustWorkforce(firm.id, firm.laborDemander, market_);
        }
        for (auto& firm : goodsFirm) {
            adjustWorkforce(firm.id, firm.laborDemander, market_);
        }

        for (auto& hhold : hholds) {
            entry(hhold.id, hhold.labor, market_);
        }

        for (auto& firm : capitalFirm) {
            offer(firm.laborDemander);
        }
        for (auto& firm : goodsFirm) {
            offer(firm.laborDemander);
        }

        for (auto& hhold : hholds) {
            accept(hhold.labor);
        }

        for (auto& firm : capitalFirm) {
            endRecruiting(firm.laborDemander, firm.capitalSupplier);
        }
        for (auto& firm : goodsFirm) {
            endRecruiting(firm.laborDemander, firm.goodsSupplier);
        }

        for (auto& hhold : hholds) {
            recordRosterEntry(hhold.labor);
        }

        for (auto& firm : capitalFirm) {
            logging(dropBox, firm.laborDemander);
        }

        for (auto& firm : goodsFirm) {
            logging(dropBox, firm.laborDemander);
        }

        for (auto& hhold : hholds) {
            logging(dropBox, hhold.labor);
        }

        market_.clear();
    }

  private:
    LaborMarket market_;
};
}  // namespace abm::engine