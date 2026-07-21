#pragma once

#include <tbb/concurrent_vector.h>
#include <atomic>

#include "config.hpp"
#include "core/base.hpp"

namespace world {

struct LaborEntry {
    const int    hholdID_;
    const double productPower_;

    bool isOffer_{false};
    bool isAccept_{false};

    LaborEntry(const int id, const double productPower)
        : hholdID_{id}, productPower_{productPower} {}
};

struct LaborRequest {
    const int    firmID_;
    const double wage_;

    tbb::concurrent_vector<LaborEntry> entryBox_;

    LaborRequest(const int id, const double wage) : firmID_{id}, wage_{wage} {}
};

struct GoodsRequest {
    const double amount_;
    double       tradeAmount_{};

    GoodsRequest(const double amount) : amount_{amount} {}
};

struct GoodsEntry {
    const double price_;
    const double supply_;

    tbb::concurrent_vector<GoodsRequest> requestBox_;

    GoodsEntry(const double price, const double supply) : price_{price}, supply_{supply} {}
};

struct Workspace {
    std::atomic<double> totalLaborInput;
    double              firmProductPower{};
};

struct RosterEntry {
    bool isOccupied_{true};
    bool isLaidOff{false};

    Workspace& workspace_;
    RosterEntry(Workspace& workspace) : workspace_{workspace} {}
};

struct CompanyBoard {
    std::vector<RosterEntry>            roster_;
    tbb::concurrent_vector<std::size_t> resignationBox_;
};

struct CensusDropBox {
    std::vector<double> firmAssets_;
    std::vector<double> postedEmployments_;
    std::vector<double> employments_;
    std::vector<double> prices_;
    std::vector<double> supplies_;
    std::vector<double> markups_;
    std::vector<double> inventories_;

    std::vector<double> hholdAssets_;
    std::vector<double> wages_;

    CensusDropBox() {
        constexpr std::size_t firmCnt{static_cast<std::size_t>(config::agent_count::firm)};
        constexpr std::size_t hholdCnt{static_cast<std::size_t>(config::agent_count::hhold)};
        firmAssets_.reserve(firmCnt);
        postedEmployments_.reserve(firmCnt);
        employments_.reserve(firmCnt);
        prices_.reserve(firmCnt);
        supplies_.reserve(firmCnt);
        markups_.reserve(firmCnt);
        inventories_.reserve(firmCnt);

        hholdAssets_.reserve(hholdCnt);
        wages_.reserve(hholdCnt);
    }

    void clear() {
        firmAssets_.clear();
        postedEmployments_.clear();
        employments_.clear();
        prices_.clear();
        supplies_.clear();
        markups_.clear();
        inventories_.clear();

        hholdAssets_.clear();
        wages_.clear();
    }
};
}  // namespace world