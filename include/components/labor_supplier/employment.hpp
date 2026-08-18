#pragma once

#include <optional>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class Employment final {
  public:
    [[nodiscard]] explicit Employment(const double productPower) : productPower_{productPower} {}

    [[nodiscard]] auto isEmployed() const -> bool { return rosterEntry_.has_value(); }

    void startWorking(RosterEntry& rosterEntry) {
        resign();
        rosterEntry_ = rosterEntry;
    }

    [[nodiscard]] auto contractFirmId() const -> AgentID {
        return isEmployed() ? rosterEntry_->firmId() : AgentID{-1};
    }
    [[nodiscard]] auto wage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        return isEmployed() ? rosterEntry_->wage : Wage{0.0};
    }

    void work(const Market phase) {
        if (shouldWork(phase)) rosterEntry_->addInput(productPower_);
    }

    [[nodiscard]] auto productPower() const -> double { return productPower_; }

    void updateStatus() {
        if (not isEmployed()) return;
        if (not rosterEntry_->isOccupied) rosterEntry_.reset();
    }

  private:
    [[nodiscard]] auto shouldWork(const Market phase) const -> bool {
        if (not isEmployed()) return false;
        return rosterEntry_->firmType() == phase;
    }

    void resign() {
        if (not isEmployed()) return;
        rosterEntry_->resign();
    }

    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    const double                productPower_;
};
}  // namespace abm::labor::supplier