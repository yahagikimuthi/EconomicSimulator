#include "strategies/labor_demander/level2.hpp"

#include <cmath>
#include <cstddef>
#include <numeric>

#include "config/contract.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor_demander {
[[nodiscard]] auto calcNextWage(const CalcNextWageCtx ctx, const double epsilonWage) -> double {
    const auto [lastWage, lastTargetEmploy, lastActualEmploy]{ctx.getLog()};
    const double occupancyRate{(lastTargetEmploy != 0) ? lastActualEmploy / lastTargetEmploy : 0.0};
    const double alpha{std::abs(helper::randNormal(0.0, ctx.wageAdjustVol(), -1.0, 1.0))};
    const double nextWage{lastWage * ((occupancyRate >= 1.0) ? 1.0 - alpha : 1.0 + alpha)};
    return std::max(epsilonWage, nextWage);
}

[[nodiscard]] auto calcNextEmploy(const CalcNextEmployCtx ctx, const bool isSold) -> int {
    const double alpha{std::abs(helper::randNormal(0.0, ctx.employAdjustVol(), -1.0, 1.0))};
    const double nextEmploy{ctx.lastTargetEmploy() * (isSold ? 1.0 - alpha : 1.0 + alpha)};
    const int    out{static_cast<int>(std::round(nextEmploy))};
    return std::max(1, out);
}

void sortApplicants(
    const int                                        employ,
    std::vector<std::size_t>&                        sortApplicantIdxs,
    const tbb::concurrent_vector<world::LaborEntry>& entryBox
) {
    const std::size_t k{std::min(entryBox.size(), static_cast<std::size_t>(employ))};
    sortApplicantIdxs.resize(entryBox.size());
    std::ranges::iota(sortApplicantIdxs, 0UZ);
    const bool isOver{entryBox.size() > static_cast<std::size_t>(employ)};
    if (not isOver) return;

    std::ranges::nth_element(
        sortApplicantIdxs,
        sortApplicantIdxs.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [&entryBox](const std::size_t idx) -> double { return ACCESS(entryBox, idx).productPower_; }
    );
}

}  // namespace labor_demander