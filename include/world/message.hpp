#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <deque>
#include <functional>
#include <optional>

#include "config.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"

namespace world {
class [[nodiscard]] Workspace {
  public:
    Workspace()  = default;
    ~Workspace() = default;
    Workspace(const Workspace& other);
    auto operator=(const Workspace& other) -> Workspace&;
    Workspace(Workspace&& other) noexcept;
    auto operator=(Workspace&& other) noexcept -> Workspace&;

    void addInput(const double workerProductPower) {
        const double input{firmProductPower * workerProductPower};
        totalInput_.fetch_add(input);
    }
    auto totalInput() const -> GoodsQuantity { return GoodsQuantity{totalInput_.load()}; }
    void resetInput() { totalInput_.store(0.0); }

    double firmProductPower{};

  private:
    std::atomic<double> totalInput_;
};

inline Workspace::Workspace(const Workspace& other)
    : firmProductPower{other.firmProductPower},
      totalInput_{std::atomic<double>{other.totalInput_.load()}} {}
inline auto Workspace::operator=(const Workspace& other) -> Workspace& {
    if (this == &other) return *this;
    firmProductPower = other.firmProductPower;
    totalInput_.store(other.totalInput_.load());
    return *this;
}
inline Workspace::Workspace(Workspace&& other) noexcept
    : firmProductPower{other.firmProductPower},
      totalInput_{std::atomic<double>{other.totalInput_.load()}} {
    other.firmProductPower = 0.0;
    other.totalInput_.store(0.0);
}
inline auto Workspace::operator=(Workspace&& other) noexcept -> Workspace& {
    if (this == &other) return *this;
    firmProductPower = other.firmProductPower;
    totalInput_.store(other.totalInput_.load());

    other.firmProductPower = 0.0;
    other.totalInput_.store(0.0);
    return *this;
}

struct [[nodiscard]] CompanyBoard {
    const AgentID                                               firmId;
    std::deque<RosterEntry>                                     roster;
    tbb::concurrent_vector<std::reference_wrapper<RosterEntry>> resignationBox;

    void resign(RosterEntry& resignEntry) { resignationBox.emplace_back(std::ref(resignEntry)); }
    CompanyBoard(const AgentID Id) : firmId{Id} {}
    auto addRoster(const AgentID id, const Wage wage, Workspace& workspace) -> RosterEntry&;
};

class [[nodiscard]] RosterEntry {
  public:
    RosterEntry(const AgentID Id, const Wage Wage, CompanyBoard& CompanyBoard, Workspace& Workspace)
        : hholdId{Id}, wage{Wage}, companyBoard{CompanyBoard}, workspace{Workspace} {}
    void addInput(const double productPower) { workspace.addInput(productPower); }
    void resign() { companyBoard.resign(*this); }
    auto firmId() const -> AgentID { return companyBoard.firmId; }

    const AgentID hholdId;
    const Wage    wage;
    bool          isOccupied{true};

  private:
    CompanyBoard& companyBoard;
    Workspace&    workspace;
};

inline auto CompanyBoard::addRoster(const AgentID id, const Wage wage, Workspace& workspace)
    -> RosterEntry& {
    return roster.emplace_back(id, wage, *this, workspace);
}

struct LaborEntry {
    const AgentID hholdID;
    const double  productPower;

    bool isOffer{false};
    bool isAccept{false};

    std::optional<RosterEntry&> rosterEntry{std::nullopt};
    const LaborRequest&         request;

    LaborEntry(const AgentID Id, const double ProductPower, const LaborRequest& Request)
        : hholdID{Id}, productPower{ProductPower}, request{Request} {}
};

struct [[nodiscard]] LaborRequest {
    LaborRequest(const AgentID Id, const Wage Wage) : firmID{Id}, wage{Wage} {}
    auto entry(const AgentID id, const double productPower) -> LaborEntry& {
        return *entryBox.emplace_back(id, productPower, *this);
    }

    const AgentID                      firmID;
    const Wage                         wage;
    tbb::concurrent_vector<LaborEntry> entryBox;
};

struct ConsumerGoodsRequest {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const ConsumerGoodsEntry& entry;
    ConsumerGoodsRequest(const GoodsQuantity Amount, const ConsumerGoodsEntry& Entry)
        : amount{Amount}, entry{Entry} {}
};

struct [[nodiscard]] ConsumerGoodsEntry {
    ConsumerGoodsEntry(const Price Price, const GoodsQuantity Supply)
        : price{Price}, supply{Supply} {}
    auto request(const GoodsQuantity amount) -> ConsumerGoodsRequest& {
        return *requestBox.emplace_back(amount, *this);
    }

    const Price                                  price;
    const GoodsQuantity                          supply;
    tbb::concurrent_vector<ConsumerGoodsRequest> requestBox;
};

struct ProductionGoodsRequest {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const ProductionGoodsEntry& entry;
    ProductionGoodsRequest(const GoodsQuantity Amount, const ProductionGoodsEntry& Entry)
        : amount{Amount}, entry{Entry} {}
};

struct [[nodiscard]] ProductionGoodsEntry {
    ProductionGoodsEntry(const Price Price, const GoodsQuantity Supply);
    auto request(const GoodsQuantity amount) -> ProductionGoodsRequest& {
        return *requestBox.emplace_back(amount, *this);
    }

    const Price                                    price;
    const GoodsQuantity                            supply;
    tbb::concurrent_vector<ProductionGoodsRequest> requestBox;
};

struct CensusDropBox {
    std::vector<double> firmAssets;
    std::vector<double> postedEmployments;
    std::vector<double> employments;
    std::vector<double> prices;
    std::vector<double> supplies;
    std::vector<double> markups;
    std::vector<double> inventories;

    std::vector<double> hholdAssets;
    std::vector<double> wages;

    CensusDropBox() {
        constexpr std::size_t firmCnt{static_cast<std::size_t>(config::agent_count::firm)};
        constexpr std::size_t hholdCnt{static_cast<std::size_t>(config::agent_count::hhold)};
        firmAssets.reserve(firmCnt);
        postedEmployments.reserve(firmCnt);
        employments.reserve(firmCnt);
        prices.reserve(firmCnt);
        supplies.reserve(firmCnt);
        markups.reserve(firmCnt);
        inventories.reserve(firmCnt);

        hholdAssets.reserve(hholdCnt);
        wages.reserve(hholdCnt);
    }

    void clear() {
        firmAssets.clear();
        postedEmployments.clear();
        employments.clear();
        prices.clear();
        supplies.clear();
        markups.clear();
        inventories.clear();

        hholdAssets.clear();
        wages.clear();
    }
};
}  // namespace world