#pragma once

#include <tbb/concurrent_vector.h>

#include "config/init_setup.hpp"

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

struct CensusDropBox {
    std::vector<double> firmAssets_;
    std::vector<int>    postedEmployments_;
    std::vector<int>    employments_;
    std::vector<double> prices_;
    std::vector<double> supplies_;
    std::vector<double> markups_;
    std::vector<double> inventory_;

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
        inventory_.reserve(firmCnt);

        hholdAssets_.reserve(hholdCnt);
        wages_.reserve(hholdCnt);
    }
};
}  // namespace world