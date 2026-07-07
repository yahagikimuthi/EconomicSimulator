#include "strategies/labor_demander/level2.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <numeric>

#include "config/contract.hpp"
#include "helper.hpp"
#include "strategies/labor_demander/level1.hpp"
#include "world/message.hpp"

namespace labor_demander {
[[nodiscard]] auto calcNextWage(const PostJobView& view, const double epsilonWage) -> double {
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