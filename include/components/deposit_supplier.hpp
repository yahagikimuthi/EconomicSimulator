#pragma once

#include <optional>

#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "world/finance.hpp"

namespace abm::deposit::supplier {
class AccountManager final {
  public:
    [[nodiscard]] constexpr AccountManager() noexcept = default;

  private:
    std::optional<DepositAccount&> account_{std::nullopt};
};

class DepositSupplier final {
    using Entry   = DepositEntry;
    using Request = DepositRequest;
    using Market  = DepositMarket;

  public:
    [[nodiscard]] explicit constexpr DepositSupplier(RandomGenerator& masterRng) noexcept;

    void requestAccountOpening(
        const AgentID id, Market& market, const int sampleCnt = setting::maxSampleCnt
    ) noexcept {
        auto picked = market.pickRequest(id, rng_, sampleCnt);
        if (not picked) return;
        myEntry_ = picked->entry(id);
    }

    void registerAccount() noexcept;

    void endStep() noexcept { reset(); }

  private:
    [[nodiscard]] auto isPosting() const noexcept -> bool { return myEntry_.has_value(); }

    void reset() noexcept { myEntry_.reset(); }

    mutable RandomGenerator rng_;
    AccountManager          accountManager_;
    std::optional<Entry&>   myEntry_{std::nullopt};
};
}  // namespace abm::deposit::supplier

namespace abm {
using DepositSupplier = deposit::supplier::DepositSupplier;
}