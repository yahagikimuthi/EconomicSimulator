#include "strategies/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <components/labor_demander.hpp>

#include "config.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor_demander {
struct CalcNextWageView {
    explicit CalcNextWageView(const PostJobView& parentComp) : comp_{parentComp.comp_} {}

    [[nodiscard]] auto log() const -> std::tuple<double, double, double> {
        const auto& log = comp_.log_;
        return {log.wage_, log.targetEmploy_, log.actualEmploy_};
    }
    [[nodiscard]] auto wageAdjustVol() const -> double {
        return comp_.parameter_.wageAdjustmentVolatility_;
    }

  private:
    Component& comp_;
};

struct CalcNextEmployView {
    explicit CalcNextEmployView(const PostJobView& parentComp_) : comp_{parentComp_.comp_} {}

    [[nodiscard]] auto employAdjustVol() const -> double {
        return comp_.parameter_.employAdjustmentVolatility_;
    }
    [[nodiscard]] auto lastTargetEmploy() const -> double { return comp_.log_.targetEmploy_; }

    [[nodiscard]] auto employeeCnt() const -> int { return comp_.log_.actualEmploy_; }

  private:
    Component& comp_;
};

namespace {
[[nodiscard]] auto calcNextWage(
    const CalcNextWageView view, const double epsilonWage = config::labor_demander::epsilonWage
) -> double {
    const auto [lastWage, lastTargetEmploy, lastActualEmploy]{view.log()};
    assert(lastWage > 0.0 && "last wage is required > 0");
    assert(lastTargetEmploy >= 0 && "last target employ is required >= 0");

    const double occupancyRate{(lastTargetEmploy != 0) ? lastActualEmploy / lastTargetEmploy : 0.0};
    const double alpha{std::abs(helper::randNormal(0.0, view.wageAdjustVol(), -1.0, 1.0))};
    const double nextWage{lastWage * ((occupancyRate >= 1.0) ? 1.0 - alpha : 1.0 + alpha)};
    return std::max(epsilonWage, nextWage);
}

[[nodiscard]] auto calcNextEmploy(const CalcNextEmployView view, const bool isSold) -> int {
    const int employeeCnt{view.employeeCnt()};
    assert(employeeCnt >= 1 && "employee count is required >= 1");
    const double alpha{
        std::abs(helper::randNormal(0.0, view.employAdjustVol(), -1.0, 1.0)) /
        std::sqrt(employeeCnt)
    };
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
    const double nextWage{calcNextWage(CalcNextWageView{view})};
    const int    nextEmploy{calcNextEmploy(CalcNextEmployView{view}, isSold)};
    view.plan(nextWage, nextEmploy);
    if (nextEmploy > 0) {
        view.posting(false);
        return;
    }
    view.posting(true);
    view.myRequest(requestBox.emplace_back(id, nextWage));
}
}  // namespace labor_demander