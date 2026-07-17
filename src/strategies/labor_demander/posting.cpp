#include "strategies/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <components/labor_demander.hpp>
#include <pcg_random.hpp>

#include "config.hpp"
#include "core/base.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor_demander {
namespace {
[[nodiscard]] auto wageGuard(
    const double wage, const double epsilon = config::labor_demander::epsilonWage
) -> double {
    return std::max(wage, epsilon);
}

struct [[nodiscard]] CalcNextWageView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto lastOfferPlan() const -> double { return comp_.log_.offerPlan_; }
    auto lastApplicantNum() const -> double { return comp_.log_.applicantNum_; }
    auto wageAdjustVol() const -> double { return comp_.parameter_.wageAdjustmentVolatility_; }
    auto lastWage() const -> double { return comp_.log_.wage_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

[[nodiscard]] auto calcNextWage(CalcNextWageView view) -> double {
    const bool   shouldRaiseWage{view.lastApplicantNum() < view.lastOfferPlan()};
    const double alpha{std::abs(helper::randNormal(view.rng(), 0.0, view.wageAdjustVol()))};
    const double nextWage{view.lastWage() * (shouldRaiseWage ? 1.0 + alpha : 1.0 - alpha)};
    return wageGuard(nextWage);
}

struct [[nodiscard]] CalcNextOfferView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto offerRate() const -> double { return comp_.parameter_.offerRate_; }
};

[[nodiscard]] auto calcNextOffer(const CalcNextOfferView& view, const int employ) -> int {
    const double offerRate{view.offerRate()};
    assert(offerRate >= 0.0 && "acceptance rate is required > 0");
    const double offer{employ * (1.0 + offerRate)};
    return static_cast<int>(std::round(offer));
}
}  // namespace

void postJob(
    const int                                    id,
    const int                                    desiredEmploy,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
) {
    const double nextWage{calcNextWage(CalcNextWageView{view})};
    const int    nextOffer{calcNextOffer(CalcNextOfferView{view}, desiredEmploy)};
    view.plan(nextWage, desiredEmploy, nextOffer);
    if (desiredEmploy == 0) {
        view.posting(false);
        return;
    }
    view.posting(true);
    view.myRequest(requestBox.emplace_back(id, nextWage));
}
}  // namespace labor_demander