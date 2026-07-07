#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>

#include "config/init_setup.hpp"
#include "core/forward.hpp"

namespace labor_supplier {
void pickSample(
    const tbb::concurrent_vector<world::LaborRequest>& requestBox,
    std::vector<std::size_t>&                          sampleIdxs,
    const int sampleCnt = config::labor_supplier::jobSampleCnt
);

void sortSample(
    const tbb::concurrent_vector<world::LaborRequest>& requestBox,
    std::vector<std::size_t>&                          sortIdxs,
    const int entryCnt = config::labor_supplier::jobEntryCnt
);
}  // namespace labor_supplier