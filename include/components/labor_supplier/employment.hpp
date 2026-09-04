#pragma once

#include <optional>

#include "components/common.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "values/labor.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class Employment final {
  public:
    explicit Employment(RandomGenerator& masterRng) noexcept
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

    template <DepositFn F>
    void work(F&& depositFn, const Date& today) noexcept {
        if (not isEmployed()) return;
        std::forward<F>(depositFn)(rosterEntry_->takeOutPaidWage());
        if (not rosterEntry_->isOccupied()) {
            rosterEntry_.reset();
            return;
        }
        if (today.day() == rosterEntry_->workDay()) rosterEntry_->addInput(productPower_);
    }

    [[nodiscard]] auto productPower() const noexcept -> double { return productPower_; }

  private:
    void resign() noexcept {
        if (not isEmployed()) return;
        rosterEntry_->resign();
    }

    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    const double                productPower_;
};
}  // namespace abm::labor::supplier