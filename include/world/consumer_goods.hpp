#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm::consumer_goods {
class ConsumerGoodsEntry;
class ConsumerGoodsRequest final {
    using Request = ConsumerGoodsRequest;
    using Entry   = ConsumerGoodsEntry;

  public:
    ConsumerGoodsRequest(const Money pay, const Entry& e) noexcept : payment{pay}, entry{e} {
        ASSERT(pay.isPositive());
    }
    ConsumerGoodsRequest(const Request&)               = delete;
    auto operator=(const Request&) -> Request&         = delete;
    ConsumerGoodsRequest(Request&&)                    = delete;
    auto operator=(ConsumerGoodsRequest&&) -> Request& = delete;
    ~ConsumerGoodsRequest() noexcept                   = default;

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

class ConsumerGoodsEntry final {
    using Request = ConsumerGoodsRequest;

  public:
    ConsumerGoodsEntry(const Price p, const GoodsQuantity s) noexcept : price{p}, supply{s} {
        ASSERT(p.isPositive());
        ASSERT(s.isPositive());
    }

    [[nodiscard]] auto request(const Money payment) noexcept -> Request& {
        ASSERT(payment.isPositive());
        return *requests_.emplace_back(payment, *this);
    }

    [[nodiscard]] auto requests() noexcept -> auto { return std::ranges::subrange{requests_}; }

    const Price         price;
    const GoodsQuantity supply;

  private:
    tbb::concurrent_vector<Request> requests_;
};

class ConsumerGoodsMarket final {
    using Entry = ConsumerGoodsEntry;

  public:
    ConsumerGoodsMarket() noexcept = default;
    [[nodiscard]] auto entry(const Price price, const GoodsQuantity supply) noexcept -> Entry& {
        ASSERT(price > Price{0.0});
        ASSERT(supply > GoodsQuantity{0.0});
        totalSupply_.fetch_add(supply.value());
        return *entries_.emplace_back(price, supply);
    }

    auto pickEntry(const int sampleCnt, RandomGenerator& rng) noexcept -> std::optional<Entry&> {
        if (entries_.empty()) return std::nullopt;
        auto toDouble = [] [[nodiscard]] (const Entry& entry) noexcept -> double {
            return entry.supply.value();
        };
        auto betterEntry =
            std::ref(rng.discreteDistribution(entries_, totalSupply_.load(), toDouble));
        if (sampleCnt <= 1) return betterEntry.get();

        for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
            auto& sample = rng.discreteDistribution(entries_, totalSupply_.load(), toDouble);
            if (sample.price >= betterEntry.get().price) continue;
            betterEntry = std::ref(sample);
        }
        return betterEntry.get();
    }

    void clear() noexcept { entries_.clear(), totalSupply_.store(0.0); }

  private:
    tbb::concurrent_vector<Entry> entries_;
    std::atomic<double>           totalSupply_;
};
}  // namespace abm::consumer_goods

namespace abm {
using ConsumerGoodsMarket = consumer_goods::ConsumerGoodsMarket;
}