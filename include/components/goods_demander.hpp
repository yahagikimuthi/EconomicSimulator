#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace goods_demander {
struct Posting {
    using GoodsMarketCoordinate =
        std::tuple<const world::GoodsEntry*, tbb::concurrent_vector<world::GoodsRequest>::iterator>;
    GoodsMarketCoordinate myRequest_;
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

  private:
    static inline int instanceCnt_{};
};
}  // namespace goods_demander