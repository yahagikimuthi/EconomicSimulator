#pragma once

#include <optional>

#include "components/common.hpp"
#include "components/labor_supplier/common.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class Employment final {
  public:
    explicit Employment(RandomGenerator& masterRng) noexcept
        : productPower_{masterRng.random(setting::productPower)} {}

    [[nodiscard]] auto isEmployed() const noexcept -> bool { return rosterEntry_.has_value(); }

    template <DepositFn F>
    void startWorking(RosterEntry& rosterEntry, F&& depositFn) noexcept {
        if (isEmployed()) {
            assert(rosterEntry_->firmId() != rosterEntry.firmId());
            assert(rosterEntry_->wage <= rosterEntry.wage);
            std::forward<F>(depositFn)(rosterEntry_->takeoutPaidWage());
        }
        resign();
        rosterEntry_ = rosterEntry;
    }

    [[nodiscard]] auto wage() const noexcept -> Wage {
        return rosterEntry_.transform(&RosterEntry::wage).value_or(Wage{0.0});
    }

    template <DepositFn F>
    void work(F&& depositFn) noexcept {
        if (not isEmployed()) return;
        std::forward<F>(depositFn)(rosterEntry_->takeoutPaidWage());
        if (not rosterEntry_->isOccupied()) {
            rosterEntry_.reset();
            return;
        }
        rosterEntry_->addInput(productPower_);
    }

    [[nodiscard]] auto makeIsAlignedRequestFn() noexcept -> IsAlignedFn auto {
        return [&] [[nodiscard]] (const Request& req) -> bool {
            if (not isEmployed()) return true;
            if (req.firmID == rosterEntry_->firmId()) return false;
            if (req.wage <= rosterEntry_->wage) return false;
            return true;
        };
    }

    [[nodiscard]] auto makeEntrySheetFn(const AgentID myId) const noexcept -> MakeEntrySheetFn
        auto {
        assert(isEmployed() ? myId == rosterEntry_->employeeId : true);
        return [&, myId] [[nodiscard]] (Request & req) -> Entry& {
            return req.entry(myId, productPower_);
        };
    }

  private:
    void resign() noexcept {
        if (not isEmployed()) return;
        rosterEntry_->resign();
        rosterEntry_.reset();
    }

    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    const double                productPower_;
};
}  // namespace abm::labor::supplier