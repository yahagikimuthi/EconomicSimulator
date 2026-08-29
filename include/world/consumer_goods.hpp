#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <atomic>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>
#include <vector>

#include "core/util.hpp"
#include "values/goods.hpp"

namespace abm::consumer_goods {
class ConsumerGoodsEntry;
class ConsumerGoodsRequest final {
  public:
    [[nodiscard]] constexpr ConsumerGoodsRequest(
        const GoodsQuantity a, const ConsumerGoodsEntry& e
    ) noexcept
        : requiresAmount{a}, entry{e} {
        ASSERT(a > GoodsQuantity{0.0});
    }
    [[nodiscard]] constexpr auto price() const noexcept -> Price;
    [[nodiscard]] auto tradeAmount() const noexcept -> GoodsQuantity { return tradeAmount_; }

    void trade(const GoodsQuantity tradeAmount) noexcept {
        ASSERT(tradeAmount >= GoodsQuantity{0.0});
        ASSERT(tradeAmount_ == GoodsQuantity{0.0});
        tradeAmount_ = tradeAmount;
    }

    const GoodsQuantity       requiresAmount;
    const ConsumerGoodsEntry& entry;

  private:
    GoodsQuantity tradeAmount_{0.0};
};

class ConsumerGoodsEntry final {
    using Request = ConsumerGoodsRequest;

  public:
    [[nodiscard]] constexpr ConsumerGoodsEntry(const Price p, const GoodsQuantity s) noexcept
        : price{p}, supply{s} {
        ASSERT(p > Price{0.0});
        ASSERT(s > GoodsQuantity{0.0});
    }
    [[nodiscard]] auto request(const GoodsQuantity amount) noexcept -> Request& {
        ASSERT(amount > GoodsQuantity{0.0});
        return *requestBox_.emplace_back(amount, *this);
    }
    void requestBox(std::vector<RefWrap<Request>>& out) noexcept {
        ASSERT(out.empty());
        for (Request& req : requestBox_) out.emplace_back(std::ref(req));
    }
    [[nodiscard]] auto totalDemand() const noexcept -> GoodsQuantity {
        const auto demand = GoodsQuantity{std::ranges::fold_left(
            requestBox_ | std::ranges::views::transform([](const Request& req) noexcept -> double {
                return req.requiresAmount.value();
            }),
            0.0,
            std::plus<>{}
        )};
        ASSERT(demand >= GoodsQuantity{0.0});
        return demand;
    }

    const Price         price;
    const GoodsQuantity supply;

    void performFullTrade() noexcept {
        for (Request& req : requestBox_) req.trade(req.requiresAmount);
    }

  private:
    tbb::concurrent_vector<Request> requestBox_;
};

class ConsumerGoodsMarket final {
    using Entry = ConsumerGoodsEntry;

  public:
    [[nodiscard]] ConsumerGoodsMarket() noexcept = default;
    [[nodiscard]] auto entry(const Price price, const GoodsQuantity supply) noexcept -> Entry& {
        ASSERT(price > Price{0.0});
        ASSERT(supply > GoodsQuantity{0.0});
        totalSupply_.fetch_add(supply.value());
        return *entryBox_.emplace_back(price, supply);
    }

    auto pickEntry(const int sampleCnt, RandomGenerator& rng) noexcept -> std::optional<Entry&> {
        if (entryBox_.empty()) return std::nullopt;
        auto toDouble = [] [[nodiscard]] (const Entry& entry) noexcept -> double {
            return entry.supply.value();
        };
        auto betterEntry =
            std::ref(rng.discreteDistribution(entryBox_, totalSupply_.load(), toDouble));
        if (sampleCnt <= 1) return betterEntry.get();

        for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
            auto& sample = rng.discreteDistribution(entryBox_, totalSupply_.load(), toDouble);
            if (sample.price >= betterEntry.get().price) continue;
            betterEntry = std::ref(sample);
        }
        return betterEntry.get();
    }

    void clear() noexcept { entryBox_.clear(), totalSupply_.store(0.0); }

  private:
    tbb::concurrent_vector<Entry> entryBox_;
    std::atomic<double>           totalSupply_;
};
}  // namespace abm::consumer_goods

namespace abm {
using ConsumerGoodsMarket = consumer_goods::ConsumerGoodsMarket;
}