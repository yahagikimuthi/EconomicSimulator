#pragma once

#include <optional>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class Employment final {
  public:
    [[nodiscard]] explicit constexpr Employment(RandomGenerator& masterRng) noexcept
        : productPower_{masterRng.random(setting::productPower)} {}

    [[nodiscard]] auto isEmployed() const noexcept -> bool { return rosterEntry_.has_value(); }

    void startWorking(RosterEntry& rosterEntry) noexcept {
        if (isEmployed()) {
            ASSERT(rosterEntry_->firmId() != rosterEntry.firmId());
            ASSERT(rosterEntry_->wage <= rosterEntry.wage);
        }
        resign();
        rosterEntry_ = rosterEntry;
    }

    [[nodiscard]] auto contractFirmId() const noexcept -> AgentID {
        return rosterEntry_.transform(&RosterEntry::firmId).value_or(AgentID{-1});
    }
    [[nodiscard]] auto wage() const noexcept -> Wage {
        return rosterEntry_.transform(&RosterEntry::wage).value_or(Wage{0.0});
    }

    void work() noexcept {
        if (not isEmployed()) return;
        rosterEntry_->addInput(productPower_);
    }

    [[nodiscard]] auto productPower() const noexcept -> double { return productPower_; }

    void updateStatus() noexcept {
        if (not isEmployed()) return;
        if (not rosterEntry_->isOccupied()) rosterEntry_.reset();
    }

  private:
    [[nodiscard]] auto shouldWork(const EMarket phase) const noexcept -> bool {
        if (not isEmployed()) return false;
        return rosterEntry_->firmType() == phase;
    }

    void resign() noexcept {
        if (not isEmployed()) return;
        rosterEntry_->resign();
    }

    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    const double                productPower_;
};
}  // namespace abm::labor::supplier