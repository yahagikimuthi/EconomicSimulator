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

struct CalcNextEmployView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto employAdjustVol() const -> double { return comp_.parameter_.employAdjustmentVolatility_; }
    auto lastEmploy() const -> double { return comp_.log_.actualEmploy_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

[[nodiscard]] auto calcNextEmploy(CalcNextEmployView view, const bool isSold) -> int {
    const double diff{std::abs(helper::randNormal(view.rng(), 0.0, view.employAdjustVol()))};
    const double employ{view.lastEmploy() + (isSold ? diff : -diff)};
    const int    out{static_cast<int>(std::round(employ))};
    return std::max(1, out);
}

struct [[nodiscard]] CalcNextOfferView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto acceptanceRate() const -> double { return comp_.parameter_.offerRate_; }
};

[[nodiscard]] auto calcNextOffer(const CalcNextOfferView& view, const int employ) -> int {
    const double acceptanceRate{view.acceptanceRate()};
    assert(acceptanceRate > 0.0 && "acceptance rate is required > 0");
    const double offer{employ / acceptanceRate};
    return static_cast<int>(std::round(offer));
}
}  // namespace

void postJob(
    const int                                    id,
    const bool                                   isSold,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
) {
    const double nextWage{calcNextWage(CalcNextWageView{view})};
    const int    nextEmploy{calcNextEmploy(CalcNextEmployView{view}, isSold)};
    const int    nextOffer{calcNextOffer(CalcNextOfferView{view}, nextEmploy)};
    view.plan(nextWage, nextEmploy, nextOffer);
    if (nextEmploy == 0) {
        view.posting(false);
        return;
    }
    view.posting(true);
    view.myRequest(requestBox.emplace_back(id, nextWage));
}
}  // namespace labor_demander