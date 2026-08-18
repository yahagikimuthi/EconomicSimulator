#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>

#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"

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
    auto totalInput() const noexcept -> GoodsQuantity { return GoodsQuantity{totalInput_.load()}; }
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
    ConsumerGoodsRequest(const GoodsQuantity Amount, const ConsumerGoodsEntry& Entry)
        : amount{Amount}, entry{Entry} {}
};

struct [[nodiscard]] ConsumerGoodsEntry final {
    ConsumerGoodsEntry(const Price Price, const GoodsQuantity Supply)
        : price{Price}, supply{Supply} {}
    auto request(const GoodsQuantity amount) -> ConsumerGoodsRequest& {
        return *requestBox.emplace_back(amount, *this);
    }

    const Price                                  price;
    const GoodsQuantity                          supply;
    tbb::concurrent_vector<ConsumerGoodsRequest> requestBox;
};

struct ProductionGoodsRequest final {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const ProductionGoodsEntry& entry;
    ProductionGoodsRequest(const GoodsQuantity Amount, const ProductionGoodsEntry& Entry)
        : amount{Amount}, entry{Entry} {}
};

struct [[nodiscard]] ProductionGoodsEntry final {
    ProductionGoodsEntry(const AgentID Id, const Price Price, const GoodsQuantity Supply)
        : id{Id}, price{Price}, supply{Supply} {}
    auto request(const GoodsQuantity amount) -> ProductionGoodsRequest& {
        return *requestBox.emplace_back(amount, *this);
    }

    const AgentID                                  id;
    const Price                                    price;
    const GoodsQuantity                            supply;
    tbb::concurrent_vector<ProductionGoodsRequest> requestBox;
};
}  // namespace abm