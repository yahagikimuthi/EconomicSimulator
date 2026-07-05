#include "strategies/labor_demander/level1.hpp"

#include <tbb/concurrent_vector.h>

#include "strategies/labor_demander/level2.hpp"
#include "world/message.hpp"

namespace labor_demander {
void postJob(
    const int                                    id,
    const bool                                   isSold,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobCtx                                   ctx,
    Component&                                   comp
) {
    const double nextWage{calcNextWage({comp})};
    const int    nextEmploy{calcNextEmploy({comp}, isSold)};
    ctx.setPlan(nextWage, nextEmploy);
    ctx.setMyRequest(requestBox.emplace_back(id, nextWage));
}
}  // namespace labor_demander