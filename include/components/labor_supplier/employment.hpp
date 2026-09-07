#pragma once

#include <optional>

#include "components/common.hpp"
#include "components/labor_supplier/common.hpp"
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

    template <DepositFn F>
    void startWorking(RosterEntry& rosterEntry, F&& depositFn) noexcept {
        if (isEmployed()) {
            ASSERT(rosterEntry_->firmId() != rosterEntry.firmId());
            ASSERT(rosterEntry_->wage <= rosterEntry.wage);
            std::forward<F>(depositFn)(rosterEntry.takeOutPaidWage());
        }
        resign();
        rosterEntry_ = rosterEntry;
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
        rosterEntry_->addInput(productPower_, today);
    }

    [[nodiscard]] auto makeIsAlignedRequestFn() noexcept -> IsAlignedFn auto {
        return [&] [[nodiscard]] (const Request& req) -> bool {
            if (not isEmployed()) return true;
            if (req.firmID == rosterEntry_->firmId()) return false;
            if (req.wage <= rosterEntry_->wage) return false;
            return true;
        };
    }

    [[nodiscard]] auto makeEntrySheetFn(const AgentID id) const noexcept -> MakeEntrySheetFn auto {
        ASSERT(isEmployed() ? id == rosterEntry_->employeeId : true);
        return [productPower = this->productPower_, id] [[nodiscard]] (Request & req) -> Entry& {
            return req.entry(id, productPower);
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