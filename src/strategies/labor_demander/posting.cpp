#include "strategies/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>

#include "config/init_setup.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor_demander {
namespace {
[[nodiscard]] auto calcNextWage(
    const PostJobView& view, const double epsilonWage = config::labor_demander::epsilonWage
) -> double {
    const auto [lastWage, lastTargetEmploy, lastActualEmploy]{view.getLog()};
    assert(lastWage > 0.0 && "last wage is required > 0");
    assert(lastTargetEmploy >= 0 && "last target employ is required >= 0");

    const double occupancyRate{(lastTargetEmploy != 0) ? lastActualEmploy / lastTargetEmploy : 0.0};
    const double alpha{std::abs(helper::randNormal(0.0, view.wageAdjustVol(), -1.0, 1.0))};
    const double nextWage{lastWage * ((occupancyRate >= 1.0) ? 1.0 - alpha : 1.0 + alpha)};
    return std::max(epsilonWage, nextWage);
}

[[nodiscard]] auto calcNextEmploy(const PostJobView& view, const bool isSold) -> int {
    const double alpha{std::abs(helper::randNormal(0.0, view.employAdjustVol(), -1.0, 1.0))};
    const double nextEmploy{view.lastTargetEmploy() * (isSold ? 1.0 - alpha : 1.0 + alpha)};
    const int    out{static_cast<int>(std::round(nextEmploy))};
    return std::max(1, out);
}
}  // namespace
void postJob(
    const int                                    id,
    const bool                                   isSold,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
) {
    const double nextWage{calcNextWage(view)};
    const int    nextEmploy{calcNextEmploy(view, isSold)};
    view.setPlan(nextWage, nextEmploy);
    view.setMyRequest(requestBox.emplace_back(id, nextWage));
}
}  // namespace labor_demander