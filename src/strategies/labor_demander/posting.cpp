#include "strategies/labor_demander/posting.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <components/labor_demander.hpp>
#include <pcg_random.hpp>

#include "helper.hpp"
#include "world/message.hpp"

namespace labor_demander::internal {
[[nodiscard]] auto wageGuard(const double wage, const double epsilon) -> double {
    return std::max(wage, epsilon);
}

[[nodiscard]] auto calcNextWage(CalcNextWageView view) -> double {
    const bool   shouldRaiseWage{view.lastApplicantNum() < view.lastOfferPlan()};
    const double alpha{std::abs(helper::randNormal(view.rng(), 0.0, view.wageAdjustVol()))};
    const double nextWage{view.lastWage() * (shouldRaiseWage ? 1.0 + alpha : 1.0 - alpha)};
    return wageGuard(nextWage);
}

[[nodiscard]] auto calcNextOffer(const CalcNextOfferView& view, const int employ) -> int {
    const double offerRate{view.offerRate()};
    assert(offerRate >= 0.0 && "acceptance rate is required > 0");
    const double offer{employ * (1.0 + offerRate)};
    return static_cast<int>(std::round(offer));
}
}  // namespace labor_demander::internal

namespace labor_demander {
void postJob(
    const int                                    id,
    const int                                    desiredEmploy,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
) {
    view.isRecruiting(true);
    const double nextWage{internal::calcNextWage(internal::CalcNextWageView{view})};
    const int nextOffer{internal::calcNextOffer(internal::CalcNextOfferView{view}, desiredEmploy)};
    view.plan(nextWage, desiredEmploy, nextOffer);
    if (desiredEmploy == 0) {
        view.posting(false);
        return;
    }
    view.posting(true);
    view.myRequest(requestBox.emplace_back(id, nextWage));
}
}  // namespace labor_demander