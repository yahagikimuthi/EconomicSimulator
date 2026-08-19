#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <functional>
#include <ranges>
#include <vector>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"

namespace abm {
class [[nodiscard]] Workspace final {
  public:
    explicit Workspace(const double power) : firmProductPower_{power} {}
    ~Workspace() = default;
    Workspace(const Workspace& other);
    auto operator=(const Workspace& other) -> Workspace&;
    Workspace(Workspace&& other) noexcept;
    auto operator=(Workspace&& other) noexcept -> Workspace&;

    void addInput(const double workerProductPower) noexcept {
        const double input{firmProductPower_ * workerProductPower};
        totalInput_.fetch_add(input);
    }
    [[nodiscard]] auto totalInput() const noexcept -> GoodsQuantity {
        return GoodsQuantity{totalInput_.load()};
    }
    void resetInput() noexcept { totalInput_.store(0.0); }

  private:
    double              firmProductPower_;
    std::atomic<double> totalInput_;
};

inline Workspace::Workspace(const Workspace& other)
    : firmProductPower_{other.firmProductPower_}, totalInput_{other.totalInput_.load()} {}
inline auto Workspace::operator=(const Workspace& other) -> Workspace& {
    if (this == &other) return *this;
    firmProductPower_ = other.firmProductPower_;
    const double input{other.totalInput_.load()};
    totalInput_.store(input);
    return *this;
}
inline Workspace::Workspace(Workspace&& other) noexcept
    : firmProductPower_{other.firmProductPower_}, totalInput_{other.totalInput_.load()} {
    other.firmProductPower_ = 0.0;
    other.totalInput_.store(0.0);
}
inline auto Workspace::operator=(Workspace&& other) noexcept -> Workspace& {
    if (this == &other) return *this;
    firmProductPower_ = other.firmProductPower_;
    const double input{other.totalInput_.load()};
    totalInput_.store(input);

    other.firmProductPower_ = 0.0;
    other.totalInput_.store(0.0);
    return *this;
}

struct ConsumerGoodsRequest final {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const ConsumerGoodsEntry& entry;
    ConsumerGoodsRequest(const GoodsQuantity a, const ConsumerGoodsEntry& e)
        : amount{a}, entry{e} {}
};

class ConsumerGoodsEntry final {
    using Request = ConsumerGoodsRequest;

  public:
    [[nodiscard]] ConsumerGoodsEntry(const Price p, const GoodsQuantity s) noexcept;
    [[nodiscard]] auto request(const GoodsQuantity amount) noexcept -> Request& {
        return *requestBox_.emplace_back(amount, *this);
    }
    void requestBox(std::vector<RefWrap<Request>>& out) noexcept {
        ASSERT(out.empty());
        for (Request& req : requestBox_) out.emplace_back(std::ref(req));
    }
    [[nodiscard]] auto totalDemand() const noexcept -> GoodsQuantity {
        const auto demand = GoodsQuantity{std::ranges::fold_left(
            requestBox_ | std::ranges::views::transform([](const Request& req) -> double {
                return req.amount.value();
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
        for (Request& req : requestBox_) req.tradeAmount = req.amount;
    }

  private:
    tbb::concurrent_vector<Request> requestBox_;
};

class ConsumerGoodsMarket final {
    using Entry = ConsumerGoodsEntry;

  public:
    [[nodiscard]] explicit ConsumerGoodsMarket(RandomGenerator& masterRng) noexcept;
    [[nodiscard]] auto entry(const Price price, const GoodsQuantity supply) noexcept -> Entry& {
        totalSupply_.fetch_add(supply.value());
        return *entryBox_.emplace_back(price, supply);
    }
    void clear() noexcept { entryBox_.clear(), totalSupply_.store(0.0); }

    auto pickEntry(const int sampleCnt) noexcept -> std::optional<Entry&> {
        if (entryBox_.empty()) return std::nullopt;
        auto toDouble    = [](const Entry& entry) -> double { return entry.supply.value(); };
        auto betterEntry = std::ref(rng_.discreteDistribution(entryBox_, totalSupply_, toDouble));
        if (sampleCnt <= 1) return betterEntry.get();

        for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
            auto& sample = rng_.discreteDistribution(entryBox_, totalSupply_, toDouble);
            if (sample.price >= betterEntry.get().price) continue;
            betterEntry = std::ref(sample);
        }
        return betterEntry.get();
    }

  private:
    tbb::concurrent_vector<Entry> entryBox_;
    RandomGenerator               rng_;
    std::atomic<double>           totalSupply_;
};

struct ProductionGoodsRequest final {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const ProductionGoodsEntry& entry;
    [[nodiscard]] ProductionGoodsRequest(const GoodsQuantity a, const ProductionGoodsEntry& e)
        : amount{a}, entry{e} {}
};

class ProductionGoodsEntry final {
    using Request = ProductionGoodsRequest;

  public:
    [[nodiscard]] ProductionGoodsEntry(const AgentID i, const Price p, const GoodsQuantity s)
        : id{i}, price{p}, supply{s} {}
    [[nodiscard]] auto request(const GoodsQuantity amount) -> Request& {
        return *requestBox_.emplace_back(amount, *this);
    }

    [[nodiscard]] auto totalDemand() const noexcept -> GoodsQuantity {
        const auto demand = GoodsQuantity{std::ranges::fold_left(
            requestBox_ | std::ranges::views::transform([](const Request& req) -> double {
                return req.amount.value();
            }),
            0.0,
            std::plus<>{}
        )};
        ASSERT(demand >= GoodsQuantity{0.0});
        return demand;
    }

    void requestBox(std::vector<RefWrap<Request>>& out) noexcept {
        ASSERT(out.empty());
        for (Request& req : requestBox_) {
            out.emplace_back(std::ref(req));
        }
    }

    void performFullTrade() {
        for (Request& req : requestBox_) req.tradeAmount = req.amount;
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
    [[nodiscard]] explicit ProductionGoodsMarket(RandomGenerator& masterRng) noexcept;
    [[nodiscard]] auto entry(
        const AgentID id, const Price price, const GoodsQuantity supply
    ) noexcept -> Entry& {
        totalSupply_.fetch_add(supply.value());
        return *entryBox_.emplace_back(id, price, supply);
    }

    auto pickEntry(const AgentID id, const int sampleCnt) noexcept -> std::optional<Entry&> {
        if (entryBox_.empty()) return std::nullopt;
        auto toDouble = [](const Entry& entry) -> double { return entry.supply.value(); };
        std::optional<Entry&> betterEntry{std::nullopt};
        for (const auto _ : std::views::iota(0, sampleCnt)) {
            auto& sample = rng_.discreteDistribution(entryBox_, totalSupply_, toDouble);
            if (sample.id == id) continue;
            if (not betterEntry) {
                betterEntry = sample;
                continue;
            }
            if (sample.price < betterEntry->price) betterEntry = sample;
        }
        return betterEntry;
    }

    void clear() { entryBox_.clear(), totalSupply_.store(0.0); }

  private:
    tbb::concurrent_vector<Entry> entryBox_;
    RandomGenerator               rng_;
    std::atomic<double>           totalSupply_;
};
}  // namespace abm