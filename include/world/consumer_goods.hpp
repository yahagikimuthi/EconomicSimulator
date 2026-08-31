#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <ranges>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm::consumer_goods {
class Entry;
class Request final {
  public:
    Request(const Money pay, const Entry& e) noexcept : payment{pay}, entry{e} {
        ASSERT(pay.isPositive());
    }
    Request(const Request&)                             = delete;
    auto operator=(const Request&) noexcept -> Request& = delete;
    Request(Request&&)                                  = delete;
    auto operator=(Request&&) noexcept -> Request&      = delete;
    ~Request() noexcept                                 = default;

    [[nodiscard]] auto price() const noexcept -> Price;
    [[nodiscard]] auto tradeAmount() const noexcept -> GoodsQuantity { return tradeAmount_; }

    void trade(const GoodsQuantity tradeAmount) noexcept {
        ASSERT(tradeAmount.isZeroOrMore());
        ASSERT(tradeAmount_.isZeroOrMore());
        tradeAmount_ = tradeAmount;
    }

    const Money  payment;
    const Entry& entry;

  private:
    GoodsQuantity tradeAmount_{0.0};
};

class Entry final {
  public:
    Entry(const AgentID Id, const Price Price, const GoodsQuantity Supply) noexcept
        : id{Id}, price{Price}, supply{Supply} {
        ASSERT(Price.isPositive());
        ASSERT(Supply.isPositive());
    }

    [[nodiscard]] auto request(const Money payment) noexcept -> Request& {
        ASSERT(payment.isPositive());
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
        ASSERT(price.isPositive());
        ASSERT(supply.isPositive());
        totalSupply_.fetch_add(supply.value());
        return *entries_.emplace_back(id, price, supply);
    }

    auto pickEntry(const AgentID id, const int sampleCnt, RandomGenerator& rng) noexcept
        -> std::optional<Entry&> {
        if (entries_.empty()) return std::nullopt;
        if (entries_.size() == 1UZ and entries_[0].id == id) return std::nullopt;

        auto currentBetter = std::optional<Entry&>{std::nullopt};
        for (const auto _ : std::views::iota(0, sampleCnt)) {
            auto& picked = rng.discreteDistribution(
                entries_,
                totalSupply_.load(),
                [](Entry& e) noexcept -> double { return e.supply.value(); }
            );
            if (picked.id == id) continue;
            if (not currentBetter) {
                currentBetter = picked;
                continue;
            }
            if (picked.price < currentBetter->price) currentBetter = picked;
        }
        return currentBetter;
    }

    void clear() noexcept { entries_.clear(), totalSupply_.store(0.0); }

  private:
    tbb::concurrent_vector<Entry> entries_;
    std::atomic<double>           totalSupply_;
};
}  // namespace abm::consumer_goods

namespace abm {
using ConsumerGoodsMarket = consumer_goods::Market;
}