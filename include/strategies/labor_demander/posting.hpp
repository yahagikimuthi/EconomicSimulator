#pragma once

#include <tbb/concurrent_vector.h>

#include "components/labor_demander.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander::internal {
[[nodiscard]] auto wageGuard(
    const double wage, const double epsilon = config::labor_demander::epsilonWage
) -> double;

struct [[nodiscard]] CalcNextWageView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto lastOfferPlan() const -> double { return comp_.log_.offerPlan_; }
    auto lastApplicantNum() const -> double { return comp_.log_.applicantNum_; }
    auto wageAdjustVol() const -> double { return comp_.parameter_.wageAdjustmentVolatility_; }
    auto lastWage() const -> double { return comp_.log_.wage_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

[[nodiscard]] auto calcNextWage(CalcNextWageView view) -> double;

struct [[nodiscard]] CalcNextOfferView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto offerRate() const -> double { return comp_.parameter_.offerRate_; }
};

[[nodiscard]] auto calcNextOffer(const CalcNextOfferView& view, const int employ) -> int;
}  // namespace labor_demander::internal

namespace labor_demander {
struct [[nodiscard]] PostJobView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void plan(const double wage, const int employ, const int offer) {
        auto& plan = comp_.plan_;
        plan.wage_ = wage, plan.employ_ = employ, plan.offer_ = offer;
    }
    void myRequest(tbb::concurrent_vector<world::LaborRequest>::iterator it) {  // NOLINT
        comp_.posting_.myRequest_ = &*it;
    }
    void posting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    void isRecruiting(const bool isRecruiting) { comp_.plan_.isRecruiting = isRecruiting; }
};

void postJob(
    const int                                    id,
    const int                                    desiredEmploy,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
);
}  // namespace labor_demander