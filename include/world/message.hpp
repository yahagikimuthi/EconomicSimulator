#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <deque>

#include "config.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"

namespace world {
struct Workspace {
    std::atomic<double> totalLaborInput;
    double              firmProductPower{};
};

struct RosterEntry {
    int    hholdId;
    double wage;
    bool   isOccupied{true};

    CompanyBoard& companyBoard;
    Workspace&    workspace;
    RosterEntry(const int Id, const double Wage, CompanyBoard& CompanyBoard, Workspace& Workspace)
        : hholdId{Id}, wage{Wage}, companyBoard{CompanyBoard}, workspace{Workspace} {}
};

struct CompanyBoard {
    const int                                    firmId;
    std::deque<RosterEntry>                      roster;
    tbb::concurrent_vector<SafePtr<RosterEntry>> resignationBox;

    CompanyBoard(const int Id) : firmId{Id} {}
};

struct LaborEntry {
    const int    hholdID;
    const double productPower;

    bool isOffer{false};
    bool isAccept{false};

    SafePtr<RosterEntry> rosterEntry{nullptr};
    const LaborRequest&  request;

    LaborEntry(const int Id, const double ProductPower, const LaborRequest& Request)
        : hholdID{Id}, productPower{ProductPower}, request{Request} {}
};

struct LaborRequest {
    const int    firmID;
    const double wage;

    tbb::concurrent_vector<LaborEntry> entryBox;

    LaborRequest(const int Id, const double Wage) : firmID{Id}, wage{Wage} {}
};

struct GoodsRequest {
    const double amount;
    double       tradeAmount{};

    const GoodsEntry& entry;
    GoodsRequest(const double Amount, const GoodsEntry& Entry) : amount{Amount}, entry{Entry} {}
};

struct GoodsEntry {
    const double price;
    const double supply;

    tbb::concurrent_vector<GoodsRequest> requestBox;

    GoodsEntry(const double Price, const double Supply) : price{Price}, supply{Supply} {}
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