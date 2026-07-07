#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>

#include "config/init_setup.hpp"
#include "core/forward.hpp"

namespace labor_demander {
struct PostJobView;
[[nodiscard]] auto calcNextWage(
    const PostJobView& view, const double epsilonWage = config::labor_demander::epsilonWage
) -> double;
[[nodiscard]] auto calcNextEmploy(const PostJobView& view, const bool isSold) -> int;

void sortApplicants(
    const int                                        employ,
    std::vector<std::size_t>&                        sortApplicantIdxs,
    const tbb::concurrent_vector<world::LaborEntry>& entryBox
);
}  // namespace labor_demander