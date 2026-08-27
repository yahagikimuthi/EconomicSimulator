#pragma once

#include <deque>
#include <optional>
#include <span>
#include <vector>

#include "components/others.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/others.hpp"
#include "world/deposit.hpp"

namespace abm::deposit::demander {
class DepositDemander final {
    using Market  = DepositMarket;
    using Request = DepositRequest;
    using Entry   = DepositEntry;

  public:
    [[nodiscard]] explicit constexpr DepositDemander(RandomGenerator& masterRng) noexcept;

    void postRequest(const AgentID id, Market& market) noexcept {
        myRequest_ = market.request(id, interestRate_);
    }

    void accountOpening() noexcept {
        if (not isPosting()) return;
        auto entries = packEntry();
        for (Entry& entry : entries) {
            auto& account = depositAccounts_.emplace_back(entry.entryAgentId);
            entry.account(account);
        }
    }

    template <AssetMinusFn F>
    void endStep(F&& assetMinus) noexcept {
        for (auto& account : depositAccounts_) {
            account.applyInterest(interestRate_);
            const auto minus = account.balance() * interestRate_;
            assetMinus(static_cast<Money>(minus));
        }
        reset();
    }

  private:
    [[nodiscard]] auto isPosting() const noexcept -> bool { return myRequest_.has_value(); }

    [[nodiscard]] auto packEntry() noexcept -> std::span<RefWrap<Entry>> {
        ASSERT(isPosting());
        static thread_local auto entries = std::vector<RefWrap<Entry>>{};
        entries.clear();
        myRequest_->packEntry(entries);
        return entries;
    }

    void reset() noexcept { myRequest_.reset(); }

    std::optional<Request&>    myRequest_;
    InterestRate               interestRate_;
    std::deque<DepositAccount> depositAccounts_;
};
}  // namespace abm::deposit::demander

namespace abm {
using DepositDemander = deposit::demander::DepositDemander;
}