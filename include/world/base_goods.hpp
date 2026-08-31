#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <optional>
#include <ranges>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm::base_goods {
class Workspace final {
  public:
    Workspace() noexcept  = default;
    ~Workspace() noexcept = default;
    Workspace(const Workspace& other) noexcept : totalInput_{other.totalInput_.load()} {}
    auto operator=(const Workspace& other) noexcept -> Workspace& {
        if (this == &other) return *this;
        const auto input = other.totalInput_.load();
        totalInput_.store(input);
        return *this;
    }
    Workspace(Workspace&& other) noexcept : totalInput_{other.totalInput_.load()} {
        other.totalInput_.store(0.0);
    }
    auto operator=(Workspace&& other) noexcept -> Workspace& {
        if (this == &other) return *this;
        const double input{other.totalInput_.load()};
        totalInput_.store(input);

        other.totalInput_.store(0.0);
        return *this;
    }

    void addInput(const double workerProductPower) noexcept {
        ASSERT(workerProductPower > 0.0);
        totalInput_.fetch_add(workerProductPower);
    }
    [[nodiscard]] auto totalInput() const noexcept -> GoodsQuantity {
        return GoodsQuantity{totalInput_.load()};
    }
    void resetInput() noexcept { totalInput_.store(0.0); }

  private:
    std::atomic<double> totalInput_;
};

class Entry;
class Request final {
  public:
    Request(const Money pay, const Entry& e) noexcept : payment_{pay}, entry_{e} {
        ASSERT(pay.isZeroOrMore());
    }
    // Entry::requests() -> std::ranges::subrangeを呼び、それに対しstd::sortを施すと
    // Requestorが持つ参照が無効化してしまう。
    // std::sortはstd::swapを内部で行い、そのコンセプトはコピー及びムーブ構築が可能であること。
    // よって、各種コンストラクタ及び演算子を明示的削除する。
    Request(const Request&)                             = delete;
    auto operator=(const Request&) noexcept -> Request& = delete;
    Request(Request&&)                                  = delete;
    auto operator=(Request&&) noexcept -> Request&      = delete;
    ~Request() noexcept                                 = default;

    [[nodiscard]] auto tradeAmount() const noexcept -> GoodsQuantity { return tradeAmount_; }
    [[nodiscard]] auto trade(const GoodsQuantity tradeAmount) noexcept -> Money;
    [[nodiscard]] auto price() const noexcept -> Price;
    [[nodiscard]] auto payment() const noexcept -> Money {
        ASSERT(payment_.isZeroOrMore());
        return payment_;
    }

  private:
    Money         payment_;
    GoodsQuantity tradeAmount_{0.0};
    const Entry&  entry_;
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

[[nodiscard]] inline auto Request::trade(const GoodsQuantity tradeAmount) noexcept -> Money {
    ASSERT(tradeAmount_.isZero());
    ASSERT(tradeAmount.isZeroOrMore());
    tradeAmount_         = tradeAmount;
    const auto actualPay = tradeAmount * entry_.price;
    payment_ -= actualPay;
    ASSERT(payment_.isZeroOrMore());
    return actualPay;
}
[[nodiscard]] inline auto Request::price() const noexcept -> Price { return entry_.price; }

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
}  // namespace abm::base_goods

namespace abm {
using BaseGoodsMarket = base_goods::Market;
}