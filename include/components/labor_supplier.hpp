#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>
#include <utility>

#include "core/base.hpp"
#include "core/forward.hpp"

namespace labor_supplier {
struct Posting {
    using LaborMarketCoordinate =
        std::pair<SafePtr<const world::LaborRequest>, SafePtr<world::LaborEntry>>;
    std::vector<LaborMarketCoordinate> myEntries_;
    bool                               isPosting_{false};
};
struct Contraction {
    int    firmID_{-1};
    double wage_{};
};
struct Parameter {
    double productPower_;
};

struct Component {
    pcg32       rng_;
    Posting     posting_;
    Contraction contraction_;
    Parameter   parameter_;

    Component(const std::uint64_t state, const std::uint64_t stream);

    [[nodiscard]] auto wage() const -> double { return contraction_.wage_; }
};
}  // namespace labor_supplier