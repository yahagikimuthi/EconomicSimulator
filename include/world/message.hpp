#pragma once

#include <tbb/concurrent_vector.h>

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

    tbb::concurrent_vector<GoodsRequest> requestBox_;

    GoodsEntry(const double price) : price_{price} {}
};
}  // namespace world