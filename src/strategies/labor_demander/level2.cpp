#include "strategies/labor_demander/level2.hpp"

#include <cmath>

#include "helper.hpp"

namespace labor_demander {
[[nodiscard]] auto calcNextWage(const CalcNextWageCtx ctx, const double epsilonWage) -> double {
    const auto [lastWage, lastTargetEmploy, lastActualEmploy]{ctx.getLog()};
    const double occupancyRate{(lastTargetEmploy != 0) ? lastActualEmploy / lastTargetEmploy : 0.0};
    const double alpha{std::abs(helper::randNormal(0.0, ctx.wageAdjustVol(), -1.0, 1.0))};
    const double nextWage{lastWage * ((occupancyRate >= 1.0) ? 1.0 - alpha : 1.0 + alpha)};
    return std::max(epsilonWage, nextWage);
}
}  // namespace labor_demander