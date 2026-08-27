#pragma once

#include <optional>

#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/others.hpp"
#include "world/deposit.hpp"

namespace abm::deposit::supplier {
class AccountManager final {
  public:
    [[nodiscard]] constexpr AccountManager() noexcept = default;

    [[nodiscard]] auto balance() const noexcept -> Deposit {
        return account_.transform(&DepositAccount::balance).value_or(Deposit{0.0});
    }

    void setAccount(DepositAccount& account) noexcept { account_ = account; }

  private:
    [[nodiscard]] auto haveAccount() const noexcept -> bool { return account_.has_value(); }
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

    void registerAccount() noexcept {
        if (not isPosting()) return;
        accountManager_.setAccount(myEntry_->account());
    }

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