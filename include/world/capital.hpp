#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <optional>
#include <ranges>

#include "others/util.hpp"
#include "values/goods.hpp"

namespace abm::capital {
class Entry;
class Request final {
  public:
    Request(const Money pay, const Entry& e) noexcept : payment{pay}, entry{e} {
        ASSERT(pay.isZeroOrMore());
    }
    Request(const Request&)                             = delete;
    auto operator=(const Request&) noexcept -> Request& = delete;
    Request(Request&&)                                  = delete;
    auto operator=(Request&&) noexcept -> Request&      = delete;
    ~Request() noexcept                                 = default;

    [[nodiscard]] auto tradeAmount() const noexcept -> GoodsQuantity { return tradeAmount_; }

    void trade(const GoodsQuantity tradeAmount) noexcept {
        ASSERT(tradeAmount_.isZero());
        ASSERT(tradeAmount.isZeroOrMore());
        tradeAmount_ = tradeAmount;
    }

    const Money  payment;
    const Entry& entry;

  private:
    GoodsQuantity tradeAmount_{0.0};
};

class Entry final {
  public:
    Entry(const AgentID i, const Price p, const GoodsQuantity s) noexcept
        : id{i}, price{p}, supply{s} {}
    [[nodiscard]] auto request(const Money payment) noexcept -> Request& {
        return *requests_.emplace_back(payment, *this);
    }

    [[nodiscard]] auto requests() noexcept -> auto { return std::ranges::subrange{requests_}; }

    const AgentID       id;
    const Price         price;
    const GoodsQuantity supply;

  private:
    tbb::concurrent_vector<Request> requests_;
};

class Market final {
  public:
    Market() noexcept = default;
    [[nodiscard]] auto entry(
        const AgentID id, const Price price, const GoodsQuantity supply
    ) noexcept -> Entry& {
        totalSupply_.fetch_add(supply.value());
        return *entries_.emplace_back(id, price, supply);
    }

    auto pickEntry(const AgentID id, const int sampleCnt, RandomGenerator& rng) noexcept
        -> std::optional<Entry&> {
        if (entries_.empty()) return std::nullopt;
        if (entries_.size() == 1UZ and entries_[0].id == id) return std::nullopt;

        auto betterEntry = std::optional<Entry&>{std::nullopt};
        for (const auto _ : std::views::iota(0, sampleCnt)) {
            auto& sample = rng.discreteDistribution(
                entries_,
                totalSupply_.load(),
                [](const Entry& e) noexcept -> double { return e.supply.value(); }
            );
            if (sample.id == id) continue;
            if (not betterEntry) {
                betterEntry = sample;
                continue;
            }
            if (sample.price < betterEntry->price) betterEntry = sample;
        }
        return betterEntry;
    }

    void clear() noexcept { entries_.clear(), totalSupply_.store(0.0); }

  private:
    tbb::concurrent_vector<Entry> entries_;
    std::atomic<double>           totalSupply_;
};
}  // namespace abm::capital

namespace abm {
using CapitalMarket = capital::Market;
}