#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <optional>
#include <ranges>
#include <utility>

#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm::base_goods {
class Workspace final {
  public:
    explicit Workspace() noexcept = default;
    ~Workspace() noexcept         = default;
    Workspace(const Workspace& other) noexcept : totalInput_{other.totalInput_.load()} {}
    auto operator=(const Workspace& other) noexcept -> Workspace& {
        if (this == &other) return *this;
        const auto input = other.totalInput_.load();
        totalInput_.store(input);
        return *this;
    }
    Workspace(Workspace&& other) noexcept              = delete;
    auto operator=(Workspace&&) noexcept -> Workspace& = delete;

    void addInput(const double workerProductPower) noexcept {
        assert(workerProductPower > 0.0);
        totalInput_.fetch_add(workerProductPower);  // TODO 処理系が対応する場合store_addに変更
    }
    [[nodiscard]] auto takeout() noexcept -> GoodsQuantity {
        const auto out = GoodsQuantity{totalInput_.load()};
        totalInput_.store(0.0);
        return out;
    }

  private:
    std::atomic<double> totalInput_;
};

class Entry;

class Request final {
  public:
    explicit Request(const Money pay, const Entry& e) noexcept
        : payment{pay}, remainPaid_{pay}, entry_{e} {
        assert(pay.isPositive());
    }
    // Entry::requests() noexcept -> std::ranges::subrangeを呼び、それに対しstd::swapを施すと
    // Requestorが持つ参照が無意味となる。
    // よって、代入演算子を明示的削除する。
    Request(const Request&)                             = default;
    auto operator=(const Request&) noexcept -> Request& = delete;
    Request(Request&&)                                  = delete;
    auto operator=(Request&&) noexcept -> Request&      = delete;
    ~Request() noexcept                                 = default;

    [[nodiscard]] auto takeoutTradeAmount() noexcept -> GoodsQuantity {
        assert(tradeAmount_.isZeroOrMore());
        return std::exchange(tradeAmount_, GoodsQuantity{0.0});
    }
    [[nodiscard]] auto trade(const GoodsQuantity tradeAmount) noexcept -> Money;
    [[nodiscard]] auto takeoutRemainPaid() noexcept -> Money {
        const auto out = std::exchange(remainPaid_, Money{0.0});
        assert(out.isZeroOrMore());
        return out;
    }

    const Money payment;

  private:
    Money         remainPaid_;
    GoodsQuantity tradeAmount_{0.0};
    const Entry&  entry_;
};

class Entry final {
  public:
    explicit Entry(const AgentID i, const Price p, const GoodsQuantity s) noexcept
        : id{i}, price{p}, supply{s} {
        assert(p.isPositive());
        assert(s.isPositive());
    }

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
    assert(tradeAmount_.isZero());
    assert(tradeAmount.isZeroOrMore());
    assert(tradeAmount <= entry_.supply);
    assert([&]() noexcept -> bool {
        const auto desired = payment / entry_.price;
        return tradeAmount <= desired;
    }());
    tradeAmount_         = tradeAmount;
    const auto actualPay = tradeAmount * entry_.price;
    remainPaid_ -= actualPay;
    assert(payment.isZeroOrMore());
    assert(remainPaid_.value() + global_setting::epsilon > 0);
    remainPaid_ = std::max(remainPaid_, Money{0.0});
    return actualPay;
}

class Market final {
  public:
    explicit Market() noexcept = default;

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

        auto       betterEntry = std::optional<Entry&>{std::nullopt};
        const auto totalSupply = totalSupply_.load();

        assert(
            totalSupply ==
            std::ranges::fold_left(
                entries_ | std::views::transform([](const Entry& e) noexcept -> double {
                    return e.supply.value();
                }),
                0.0,
                std::plus{}
            )
        );

        for (const auto _ : std::views::indices(sampleCnt)) {
            auto& sample = rng.discreteDistribution(
                entries_,
                totalSupply,
                [](const Entry& e) noexcept -> double { return e.supply.value(); }
            );
            if (sample.id == id) continue;
            if (not betterEntry or sample.price < betterEntry->price) betterEntry = sample;
        }

        return betterEntry;
    }

    void clear() noexcept {
        entries_.clear();
        totalSupply_.store(0.0);
    }

  private:
    tbb::concurrent_vector<Entry> entries_;
    std::atomic<double>           totalSupply_;
};
}  // namespace abm::base_goods

namespace abm::capital {
using Entry   = base_goods::Entry;
using Request = base_goods::Request;
using Market  = base_goods::Market;
}  // namespace abm::capital

namespace abm::goods {
using Entry   = base_goods::Entry;
using Request = base_goods::Request;
using Market  = base_goods::Market;
}  // namespace abm::goods

namespace abm {
using GoodsMarket   = goods::Market;
using CapitalMarket = goods::Market;
}  // namespace abm