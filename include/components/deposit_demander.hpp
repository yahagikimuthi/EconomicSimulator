#pragma once

#include <deque>
#include <optional>
#include <span>
#include <vector>

#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/others.hpp"
#include "world/finance.hpp"

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
            auto& account = savingAccounts_.emplace_back(entry.entryAgentId);
            entry.account(account);
        }
    }

    void endStep() noexcept { reset(); }

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

    std::optional<DepositRequest&> myRequest_;
    InterestRate                   interestRate_;
    std::deque<DepositAccount>     savingAccounts_;
};
}  // namespace abm::deposit::demander

namespace abm {
using DepositDemander = deposit::demander::DepositDemander;
}