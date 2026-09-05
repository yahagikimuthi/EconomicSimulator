#pragma once

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <atomic>
#include <memory>
#include <optional>
#include <ranges>

#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/date.hpp"
#include "values/goods.hpp"
#include "world/common.hpp"

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
    using EntryT = Entry<MarketT>;
    explicit Request(const Money pay, const EntryT& e) noexcept
        : payment_{pay}, remainPaid_{pay}, entry_{e} {
        ASSERT(pay.isZeroOrMore());
    }
    // Entry::requests() -> std::ranges::subrangeを呼び、それに対しstd::swapを施すと
    // Requestorが持つ参照が無意味となる。
    // よって、代入演算子を明示的削除する。
    Request(const Request&)                             = default;
    auto operator=(const Request&) noexcept -> Request& = default;
    Request(Request&&)                                  = delete;
    auto operator=(Request&&) noexcept -> Request&      = delete;
    ~Request() noexcept                                 = default;

    [[nodiscard]] auto tradeAmount() const noexcept -> GoodsQuantity { return tradeAmount_; }
    [[nodiscard]] auto trade(const GoodsQuantity tradeAmount) noexcept -> Money;
    [[nodiscard]] auto payment() const noexcept -> Money {
        ASSERT(payment_.isZeroOrMore());
        return payment_;
    }
    [[nodiscard]] auto remainPaid() const noexcept -> Money { return remainPaid_; }

  private:
    const Money   payment_;
    Money         remainPaid_;
    GoodsQuantity tradeAmount_{0.0};
    const EntryT& entry_;
};

template <EMarket MarketT>
    requires(MarketT == EMarket::Capital or MarketT == EMarket::Goods)
class Market;
template <EMarket MarketT>
class Entry final {
    using RequestT = Request<MarketT>;

  public:
    explicit Entry(
        const AgentID i, const Price p, const GoodsQuantity s, Market<MarketT>& market
    ) noexcept
        : id{i}, price{p}, supply{s}, market_{market} {}

    [[nodiscard]] auto request(const Money payment) noexcept -> RequestT& {
        return *requests_.emplace_back(payment, *this);
    }

    [[nodiscard]] auto requests() noexcept -> auto { return std::ranges::subrange{requests_}; }
    [[nodiscard]] auto isValid() const noexcept -> bool { return isValid_; }

    void disable() noexcept;

    const AgentID       id;
    const Price         price;
    const GoodsQuantity supply;

  private:
    tbb::concurrent_vector<RequestT> requests_;
    Market<MarketT>&                 market_;
    bool                             isValid_{true};
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
    requires(MarketT == EMarket::Capital or MarketT == EMarket::Goods)
class Market final {
    using EntryT = Entry<MarketT>;

    struct EmptyEntry final {
        explicit EmptyEntry(EntryT& Entry, const Day DisableDay) noexcept
            : entry{Entry}, disableDay{DisableDay} {}
        explicit EmptyEntry() noexcept = default;
        std::optional<EntryT&> entry{std::nullopt};
        Day                    disableDay{1};
    };

  public:
    explicit Market(const Date& today) noexcept : today_{today} {}

    [[nodiscard]] auto entry(
        const AgentID id, const Price price, const GoodsQuantity supply
    ) noexcept -> EntryT& {
        totalSupply_.fetch_add(supply.value());  // TODO 処理系が対応する場合store_addに変更
        auto       newEntry = EmptyEntry{};
        const auto result   = emptyEntries_.try_pop(newEntry);
        if (not result or not canReuse(newEntry.disableDay))
            return *entries_.emplace_back(id, price, supply, *this);

        ASSERT(newEntry.entry);
        ASSERT(not newEntry.entry->isValid());
        std::destroy_at(&*newEntry.entry);
        return *std::construct_at(&*newEntry.entry, id, price, supply, *this);
    }

    auto pickEntry(const AgentID id, const int sampleCnt, RandomGenerator& rng) noexcept
        -> std::optional<EntryT&> {
        if (entries_.empty()) return std::nullopt;
        if (entries_.size() == 1UZ and entries_[0].id == id) return std::nullopt;

        auto betterEntry = std::optional<EntryT&>{std::nullopt};
        for (const auto _ : std::views::indices(sampleCnt)) {
            auto& sample = rng.discreteDistribution(
                entries_ | std::views::filter(&EntryT::isValid),
                totalSupply_.load(),
                [](const EntryT& e) noexcept -> double { return e.supply.value(); }
            );
            if (sample.id == id) continue;
            if (not betterEntry or sample.price < betterEntry->price) betterEntry = sample;
        }
        return betterEntry;
    }

    void disable(EntryT& entry) noexcept {
        emptyEntries_.emplace(entry, today_.day());
        totalSupply_.fetch_sub(entry.supply.value());
    }

    [[nodiscard]] auto canReuse(const Day disableDay) const noexcept -> bool {
        ASSERT(disableDay < Day{global_setting::dayInMonth});
        if (disableDay == today_.day()) return false;
        if (disableDay + Day{1} == today_.day()) return false;
        return true;
    }

  private:
    tbb::concurrent_vector<EntryT>    entries_;
    tbb::concurrent_queue<EmptyEntry> emptyEntries_;
    std::atomic<double>               totalSupply_;
    const Date&                       today_;
};

template <EMarket MarketT>
inline void Entry<MarketT>::disable() noexcept {
    isValid_ = false;
    market_.disable(*this);
}
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