#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace goods_demander {
struct Posting {
    using GoodsMarketCoordinate = std::tuple<const world::GoodsEntry*, world::GoodsRequest*>;
    GoodsMarketCoordinate myRequest_{nullptr, nullptr};
    bool                  isPosting_{false};
};
struct Purchasing {
    double purchase_{};
};
struct Parameter {
    const double mpc_;
    const int    myPhase_;
};
struct Component {
    Posting    posting_;
    Purchasing purchasing_;
    Parameter  parameter_;

    Component();
    void reset();

  private:
    static inline int instanceCnt_{};
};
}  // namespace goods_demander