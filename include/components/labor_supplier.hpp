#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>
#include <utility>
#include <world/message.hpp>

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
    const bool  isEmploying_;
    std::size_t contractFirmIdx_;
    std::size_t myRosterEntryIdx_;
};
struct Parameter {
    double productPower_;
};

struct [[nodiscard]] Component {
    pcg32       rng_;
    Posting     posting_;
    Contraction contraction_;
    Parameter   parameter_;

    Component(const std::uint64_t state, const std::uint64_t stream);

    auto wage(const std::span<const world::CompanyBoard> companyBoards) const -> double {
        if (not contraction_.isEmploying_) return 0.0;
        return companyBoards[contraction_.contractFirmIdx_]
            .roster_[contraction_.myRosterEntryIdx_]
            .wage_;
    }
};
}  // namespace labor_supplier