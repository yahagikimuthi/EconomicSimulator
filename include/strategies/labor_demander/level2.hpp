#pragma once

#include <tuple>

#include "components/labor_demander.hpp"

namespace labor_demander {
struct CalcNextWageCtx {
    CalcNextWageCtx(Component& component) : comp_{component} {}

    [[nodiscard]] auto getLog() const -> std::tuple<double, double, double> {
        return {comp_.log_.wage_, comp_.log_.targetEmploy_, comp_.log_.actualEmploy_};
    }
    [[nodiscard]] auto wageAdjustVol() const -> double {
        return comp_.parameter_.wageAdjustmentVolatility_;
    }

  private:
    Component& comp_;
};

[[nodiscard]] auto calcNextWage(const CalcNextWageCtx ctx, const double epsilonWage) -> double;

struct CalcNextEmployCtx {
    CalcNextEmployCtx(Component& component) : comp_{component} {}

    [[nodiscard]] auto lastTargetEmploy() const -> double { return comp_.log_.targetEmploy_; }
    [[nodiscard]] auto employAdjustVol() const -> double {
        return comp_.parameter_.employAdjustmentVolatility_;
    }

  private:
    Component& comp_;
};

[[nodiscard]] auto calcNextEmploy(const CalcNextEmployCtx ctx, const bool isSold) -> int;
}  // namespace labor_demander