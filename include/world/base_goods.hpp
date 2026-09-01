#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <optional>
#include <ranges>

#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/common.hpp"

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
        totalInput_.fetch_add(workerProductPower);  // TODO 処理系が対応する場合store_addに変更
    }
    [[nodiscard]] auto totalInput() const noexcept -> GoodsQuantity {
        return GoodsQuantity{totalInput_.load()};
    }
    void resetInput() noexcept { totalInput_.store(0.0); }

  private:
    std::atomic<double> totalInput_;
};

template <EMarket MarketT>
class Entry;

template <EMarket MarketT>
class Request final {
  public:
    using Entry = Entry<MarketT>;
    Request(const Money pay, const Entry& e) noexcept : payment_{pay}, remainPaid_{pay}, entry_{e} {
        ASSERT(pay.isZeroOrMore());
    }
    // Entry::requests() -> std::ranges::subrangeを呼び、それに対しstd::swapを施すと
    // Requestorが持つ参照が無意味となる。
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
    [[nodiscard]] auto remainPaid() const noexcept -> Money { return remainPaid_; }

  private:
    const Money   payment_;
    Money         remainPaid_;
    GoodsQuantity tradeAmount_{0.0};
    const Entry&  entry_;
};

template <EMarket MarketT>
class Entry final {
    using Request = Request<MarketT>;

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

template <EMarket MarketT>
[[nodiscard]] inline auto Request<MarketT>::trade(const GoodsQuantity tradeAmount
) noexcept -> Money {
    ASSERT(tradeAmount_.isZero());
    ASSERT(tradeAmount.isZeroOrMore());
    tradeAmount_         = tradeAmount;
    const auto actualPay = tradeAmount * entry_.price;
    remainPaid_ -= actualPay;
    ASSERT(payment_.isZeroOrMore());
    return actualPay;
}

template <EMarket MarketT>
[[nodiscard]] inline auto Request<MarketT>::price() const noexcept -> Price {
    return entry_.price;
}

template <EMarket MarketT>
    requires(MarketT == EMarket::Capital or MarketT == EMarket::Goods)
class Market final {
    using Entry = Entry<MarketT>;

  public:
    Market() noexcept = default;
    [[nodiscard]] auto entry(
        const AgentID id, const Price price, const GoodsQuantity supply
    ) noexcept -> Entry& {
        totalSupply_.fetch_add(supply.value());  // TODO 処理系が対応する場合store_addに変更
        return *entries_.emplace_back(id, price, supply);
    }

    auto pickEntry(const AgentID id, const int sampleCnt, RandomGenerator& rng) noexcept
        -> std::optional<Entry&> {
        if (entries_.empty()) return std::nullopt;
        if (entries_.size() == 1UZ and entries_[0].id == id) return std::nullopt;

        auto betterEntry = std::optional<Entry&>{std::nullopt};
        for (const auto _ : std::views::indices(sampleCnt)) {
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

namespace abm::goods {
using Market  = base_goods::Market<EMarket::Goods>;
using Entry   = base_goods::Entry<EMarket::Goods>;
using Request = base_goods::Request<EMarket::Goods>;
}  // namespace abm::goods

namespace abm::capital {
using Market  = base_goods::Market<EMarket::Capital>;
using Entry   = base_goods::Entry<EMarket::Capital>;
using Request = base_goods::Request<EMarket::Capital>;
}  // namespace abm::capital

namespace abm {
using GoodsMarket   = goods::Market;
using CapitalMarket = capital::Market;
}  // namespace abm