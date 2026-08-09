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
struct [[nodiscard]] Workspace {
  public:
    double firmProductPower{};

    void addInput(const double workerProductPower) {
        totalInput_ += firmProductPower * workerProductPower;
    }
    auto totalInput() const -> GoodsQuantity { return GoodsQuantity{totalInput_}; }
    void resetInput() { totalInput_ = 0.0; }

  private:
    std::atomic<double> totalInput_;
};

struct CompanyBoard {
    const AgentID                                               firmId;
    std::deque<RosterEntry>                                     roster;
    tbb::concurrent_vector<std::reference_wrapper<RosterEntry>> resignationBox;

    void resign(RosterEntry& resignEntry) { resignationBox.emplace_back(std::ref(resignEntry)); }
    CompanyBoard(const AgentID Id) : firmId{Id} {}
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

struct LaborRequest {
    const AgentID                      firmID;
    const Wage                         wage;
    tbb::concurrent_vector<LaborEntry> entryBox;

    LaborRequest(const AgentID Id, const Wage Wage) : firmID{Id}, wage{Wage} {}
};

struct GoodsRequest {
    const GoodsQuantity amount;
    GoodsQuantity       tradeAmount{0.0};

    const GoodsEntry& entry;
    GoodsRequest(const GoodsQuantity Amount, const GoodsEntry& Entry)
        : amount{Amount}, entry{Entry} {}
};

struct GoodsEntry {
    const Price         price;
    const GoodsQuantity supply;

    tbb::concurrent_vector<GoodsRequest> requestBox;

    GoodsEntry(const Price Price, const GoodsQuantity Supply) : price{Price}, supply{Supply} {}
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