#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <vector>

#include "core/assertion.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/others.hpp"

namespace abm {
class DepositAccount final {
  public:
    [[nodiscard]] explicit constexpr DepositAccount(const AgentID id) noexcept : depositorId{id} {}

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
class DepositEntry final {
  public:
    [[nodiscard]] explicit constexpr DepositEntry(const AgentID id, const DepositRequest& req)
        : entryAgentId{id}, request{req} {}

    void account(DepositAccount& account) noexcept { account_ = account; }

    [[nodiscard]] auto account() noexcept -> DepositAccount& {
        ASSERT(account_);
        return *account_;
    }

    const AgentID         entryAgentId;
    const DepositRequest& request;

  private:
    std::optional<DepositAccount&> account_{std::nullopt};
};

class DepositRequest final {
    using Entry = DepositEntry;

  public:
    [[nodiscard]] constexpr DepositRequest(const AgentID id, const InterestRate rate)
        : bankId{id}, interestRate{rate} {}

    [[nodiscard]] auto entry(const AgentID entryAgentId) noexcept -> Entry& {
        ASSERT(entryAgentId != bankId);
        return *entries_.emplace_back(entryAgentId, *this);
    }
    void packEntry(std::vector<RefWrap<Entry>>& out) noexcept {
        ASSERT(out.empty());
        for (auto& entry : entries_) out.emplace_back(std::ref(entry));
    }

    const AgentID      bankId;
    const InterestRate interestRate;

  private:
    tbb::concurrent_vector<Entry> entries_;
};

class DepositMarket final {
    using Request = DepositRequest;

  public:
    [[nodiscard]] constexpr DepositMarket() noexcept = default;

    [[nodiscard]] auto request(const AgentID bankId, const InterestRate interestRate) noexcept
        -> Request& {
        return *requests_.emplace_back(bankId, interestRate);
    }

    [[nodiscard]] auto pickRequest(
        const AgentID id, RandomGenerator& rng, const int maxSampleCnt
    ) noexcept -> std::optional<Request&> {
        if (requests_.empty()) return std::nullopt;
        if (requests_.size() == 1UZ and requests_[0].bankId == id) return std::nullopt;

        for (const auto _ : std::views::iota(0, maxSampleCnt)) {
            const auto maxIdx = static_cast<int>(requests_.size() - 1UZ);
            const auto idx    = rng.randInt(0, maxIdx);
            auto&      sample = requests_[static_cast<std::size_t>(idx)];
            if (sample.bankId != id) return sample;
        }
        return std::nullopt;
    }

    void clear() noexcept { requests_.clear(); }

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