#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>

#include "core/forward.hpp"

namespace labor_demander {
struct Log {
    double wage_;
    int    targetEmploy_;
    int    actualEmploy_;
};
struct Plan {
    double wage_;
    int    employ_;
};
struct HR {
    double sumWage_;
    double sumEmployeeProductPower_;
    int    employeeCnt;
};
struct Parameter {
    const double wageAdjustmentVolatility_;
    const double employAdjustmentVolatility_;
};
struct Posting {
    tbb::concurrent_vector<world::LaborRequest>::iterator myBoxIdx_;
    std::vector<std::size_t>                              offerApplicantIdxs_;
    bool                                                  isPosting_;
};
struct Component {
    Log       log_;
    Plan      plan_;
    Posting   posting_;
    HR        humanResources;
    Parameter parameter_;
    Component();
};
}  // namespace labor_demander