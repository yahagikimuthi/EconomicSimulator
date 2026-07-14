#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>

#include "core/forward.hpp"

namespace goods_supplier {
struct Log {
    double markup_;
    double price_;
    double supply_;
    double sales_;
    bool   isSold_;
};
struct Plan {
    double markup_;
    double price_;
    double supply_;
};
struct SalesLedger {
    double inventory_;
    double currentSales;
};
struct Posting {
    world::GoodsEntry* myEntry_{nullptr};
    bool               isPosting_{false};
};
struct Production {
    double firmProductPower_;
    double sumEmployeeProductPower_;
    double inventory_;
};
struct Parameter {
    const double targetInventoryRatio_;
    const double markupAdjustmentVolatility_;
};
struct Component {
    pcg32       rng_;
    Log         log_{};
    Plan        plan_{};
    SalesLedger salesLedger{};
    Posting     posting_;
    Production  production_;
    Parameter   parameter_;

    Component(const std::uint64_t state, const std::uint64_t stream);
    void setSumEmployeeProductPower(const double power) {
        production_.sumEmployeeProductPower_ = power;
    }
    [[nodiscard]] auto isSold() const -> bool { return log_.isSold_; }
    [[nodiscard]] auto sales() const -> double { return salesLedger.currentSales; }
};
}  // namespace goods_supplier