#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>
#include <world/message.hpp>

#include "core/base.hpp"
#include "core/forward.hpp"

namespace labor_supplier {
struct Posting {
    std::vector<SafePtr<world::LaborEntry>> myEntries_;
    SafePtr<const world::LaborEntry>        acceptEntry_;
    bool                                    isPosting_{false};
};
struct Parameter {
    double productPower_;
};

struct [[nodiscard]] Component {
    pcg32                       rng_;
    Posting                     posting_;
    SafePtr<world::RosterEntry> rosterEntry_{nullptr};
    double                      productPower_;

    Component(const std::uint64_t state, const std::uint64_t stream);

    auto wage() const -> double { return (rosterEntry_) ? rosterEntry_->wage_ : 0.0; }
    auto acceptedEntry() const -> const world::LaborEntry& { return *posting_.acceptEntry_; }
    void rosterEntry(const SafePtr<world::RosterEntry> rosterEntry) { rosterEntry_ = rosterEntry; }
    auto isEmploying() const -> bool { return rosterEntry_.hasValue(); }
};
}  // namespace labor_supplier