#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace goods_supplier {
struct Log {
    double markup_;
    double price_;
    double supply_;
    double sales_;
    double expectedDemand_;
    bool   isSold_;
};
struct Plan {
    double markup_;
    double price_;
    double supply_;
};
struct Posting {
    tbb::concurrent_vector<world::GoodsEntry>::iterator myEntry_;
    bool                                                isPosting_{false};
};
struct Production {
    double firmProductPower_;
    double sumEmployeeProductPower_;
    double inventory_;
};
struct Parameter {
    const double targetInventoryRatio_;
    const double markupAdjustmentVolatility_;
    const double demandAdaptionRate_;
};
struct Component {
    Log        log_;
    Plan       plan_;
    Posting    posting_;
    Production production_;
    Parameter  parameter_;

    Component();

    void setSumEmployeeProductPower(const double power) {
        production_.sumEmployeeProductPower_ = power;
    }
    [[nodiscard]] auto getIsSold() const -> bool { return log_.isSold_; }
};
}  // namespace goods_supplier