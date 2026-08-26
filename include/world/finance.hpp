#pragma once

#include <tbb/concurrent_vector.h>
#include <iterator>
#include <optional>
#include <ranges>
#include <vector>

#include "core/util.hpp"
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
    [[nodiscard]] explicit constexpr DepositEntry(const AgentID id, const DepositRequest& req)
        : entryAgentId{id}, request{req} {}

    void account(SavingAccount& account) noexcept { account_ = account; }

    [[nodiscard]] auto account() noexcept -> SavingAccount& {
        ASSERT(account_);
        return *account_;
    }

    const AgentID         entryAgentId;
    const DepositRequest& request;

  private:
    std::optional<SavingAccount&> account_{std::nullopt};
};

class DepositRequest {
    using Entry = DepositEntry;

  public:
    [[nodiscard]] constexpr DepositRequest(const AgentID id, const InterestRate rate)
        : bankId{id}, interestRate{rate} {}

    [[nodiscard]] auto entry(const AgentID entryAgentId) noexcept -> Entry& {
        ASSERT(entryAgentId != bankId);
        return *entries.emplace_back(entryAgentId, *this);
    }

    const AgentID      bankId;
    const InterestRate interestRate;

  private:
    tbb::concurrent_vector<Entry> entries;
};

class DepositMarket {
    using Request = DepositRequest;

  public:
    [[nodiscard]] constexpr DepositMarket() noexcept = default;

    [[nodiscard]] auto request(const AgentID bankId, const InterestRate interestRate) noexcept
        -> Request& {
        return *requests_.emplace_back(bankId, interestRate);
    }

    void packRequest(
        const AgentID                  id,
        const int                      sampleCnt,
        std::vector<RefWrap<Request>>& out,
        RandomGenerator&               rng
    ) noexcept {
        ASSERT(out.empty());
        if (sampleCnt >= static_cast<int>(requests_.size())) {
            packAllRequest(id, out);
            return;
        }
        packPartRequest(id, sampleCnt, out, rng);
    }

  private:
    void packAllRequest(const AgentID id, std::vector<RefWrap<Request>>& out) noexcept {
        for (auto& req : requests_) {
            if (req.bankId == id) continue;
            out.emplace_back(req);
        }
    }

    void packPartRequest(
        const AgentID                  id,
        const int                      sampleCnt,
        std::vector<RefWrap<Request>>& out,
        RandomGenerator&               rng
    ) noexcept {
        rng.sample(
            requests_ |
                std::views::filter([id](const Request& req) -> bool { return req.bankId != id; }),
            std::back_inserter(out),
            sampleCnt
        );
    }

    tbb::concurrent_vector<Request> requests_;
};
}  // namespace abm