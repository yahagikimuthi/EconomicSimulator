#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>

#include "core/values/common.hpp"
#include "core/values/others.hpp"

namespace abm {
class SavingAccount {
  public:
    [[nodiscard]] explicit constexpr SavingAccount(const AgentID id) noexcept : depositorId{id} {}

    const AgentID depositorId;

    void disable() noexcept {
        isValid  = false;
        deposit_ = Deposit{0.0};
    }

    void applyInterest(const InterestRate rate) noexcept {
        ASSERT(InterestRate{0.0} < rate and rate < InterestRate{1.0});
        deposit_ += deposit_ * (InterestRate{1.0} + rate);
    }

  private:
    bool    isValid{true};
    Deposit deposit_{0.0};
};

class DepositRequest;
class DepositEntry {
  public:
    [[nodiscard]] explicit constexpr DepositEntry(const AgentID id);

    void account(SavingAccount& account) noexcept { account_ = account; }

    [[nodiscard]] auto account() noexcept -> SavingAccount& {
        ASSERT(account_);
        return *account_;
    }

    const AgentID entryAgentId;

  private:
    std::optional<SavingAccount&> account_{std::nullopt};
};

class DepositRequest {
  public:
    [[nodiscard]] constexpr DepositRequest();

    const AgentID      bankId;
    const InterestRate interestRate;

  private:
    tbb::concurrent_vector<DepositEntry> entryBox_;
};
}  // namespace abm