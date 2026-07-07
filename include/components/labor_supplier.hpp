#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>

#include "core/forward.hpp"

namespace labor_supplier {
struct Posting {
    using LaborMarketCoordinate = std::pair<
        std::reference_wrapper<world::LaborRequest>,
        tbb::concurrent_vector<world::LaborEntry>::iterator>;

    std::vector<LaborMarketCoordinate> myEntries_;
    bool                               isPosting_;
};
struct Contraction {
    int    firmID_;
    double wage_;
};
struct Parameter {
    double productPower_;
};

struct Component {
    Posting     posting_;
    Contraction contraction_;
    Parameter   parameter_;

    Component();
};
}  // namespace labor_supplier