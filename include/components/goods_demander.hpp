#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace goods_demander {
struct Posting {
    tbb::concurrent_vector<world::GoodsRequest>::iterator myRequestIdx;
    bool                                                  isPosting_;
};
struct Purchasing {
    double purchase_;
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
};
}  // namespace goods_demander