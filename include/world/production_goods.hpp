#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <functional>
#include <optional>
#include <ranges>
#include <vector>

#include "core/util.hpp"
#include "core/values/goods.hpp"

namespace abm::production_goods {
class ProductionGoodsEntry;
class ProductionGoodsRequest final {
  public:
    [[nodiscard]] constexpr ProductionGoodsRequest(
        const GoodsQuantity a, const ProductionGoodsEntry& e
    ) noexcept
        : amount{a}, entry{e} {
        ASSERT(a >= GoodsQuantity{0.0});
    }
    [[nodiscard]] auto tradeAmount() const noexcept -> GoodsQuantity { return tradeAmount_; }

    void trade(const GoodsQuantity tradeAmount) noexcept {
        ASSERT(tradeAmount_ == GoodsQuantity{0.0});
        ASSERT(tradeAmount >= GoodsQuantity{0.0});
        tradeAmount_ = tradeAmount;
    }

    const GoodsQuantity         amount;
    const ProductionGoodsEntry& entry;

  private:
    GoodsQuantity tradeAmount_{0.0};
};

class ProductionGoodsEntry final {
    using Request = ProductionGoodsRequest;

  public:
    [[nodiscard]] constexpr ProductionGoodsEntry(
        const AgentID i, const Price p, const GoodsQuantity s
    ) noexcept
        : id{i}, price{p}, supply{s} {}
    [[nodiscard]] auto request(const GoodsQuantity amount) noexcept -> Request& {
        return *requestBox_.emplace_back(amount, *this);
    }

    [[nodiscard]] auto totalDemand() const noexcept -> GoodsQuantity {
        const auto demand = GoodsQuantity{std::ranges::fold_left(
            requestBox_ | std::ranges::views::transform([](const Request& req) noexcept -> double {
                return req.amount.value();
            }),
            0.0,
            std::plus<>{}
        )};
        ASSERT(demand >= GoodsQuantity{0.0});
        return demand;
    }

    void packRequest(std::vector<RefWrap<Request>>& out) noexcept {
        ASSERT(out.empty());
        for (Request& req : requestBox_) {
            out.emplace_back(std::ref(req));
        }
    }

    void performFullTrade() noexcept {
        for (Request& req : requestBox_) req.trade(req.amount);
    }

    const AgentID       id;
    const Price         price;
    const GoodsQuantity supply;

  private:
    tbb::concurrent_vector<Request> requestBox_;
};

class ProductionGoodsMarket final {
    using Entry = ProductionGoodsEntry;

  public:
    [[nodiscard]] ProductionGoodsMarket() noexcept = default;
    [[nodiscard]] auto entry(
        const AgentID id, const Price price, const GoodsQuantity supply
    ) noexcept -> Entry& {
        totalSupply_.fetch_add(supply.value());
        return *entryBox_.emplace_back(id, price, supply);
    }

    auto pickEntry(const AgentID id, const int sampleCnt, RandomGenerator& rng) noexcept
        -> std::optional<Entry&> {
        if (entryBox_.empty()) return std::nullopt;
        auto toDouble = [] [[nodiscard]] (const Entry& entry) noexcept -> double {
            return entry.supply.value();
        };
        std::optional<Entry&> betterEntry{std::nullopt};
        for (const auto _ : std::views::iota(0, sampleCnt)) {
            auto& sample = rng.discreteDistribution(entryBox_, totalSupply_.load(), toDouble);
            if (sample.id == id) continue;
            if (not betterEntry) {
                betterEntry = sample;
                continue;
            }
            if (sample.price < betterEntry->price) betterEntry = sample;
        }
        return betterEntry;
    }

    void clear() noexcept { entryBox_.clear(), totalSupply_.store(0.0); }

  private:
    tbb::concurrent_vector<Entry> entryBox_;
    std::atomic<double>           totalSupply_;
};
}  // namespace abm::production_goods

namespace abm {
using ProductionGoodsMarket = production_goods::ProductionGoodsMarket;
}