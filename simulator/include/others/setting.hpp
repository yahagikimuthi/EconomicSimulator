#pragma once

#include <limits>
#include <variant>

#include "others/type.hpp"

namespace abm {
template <typename T>
struct UniformParameter final {
    const T min;
    const T limit;
};

struct NormalParameter final {
    const f64 mean{0.0};
    const f64 dev{1.0};
    const f64 min{-std::numeric_limits<f64>::infinity()};
    const f64 max{std::numeric_limits<f64>::infinity()};
};

using RandomParameter = std::variant<UniformParameter<i32>, UniformParameter<f64>, NormalParameter>;

struct PCG32Seed final {
    const u64 state;
    const u64 stream;
};
}  // namespace abm

namespace abm::global_setting {
inline constexpr auto useRuntimeRandomSeed = false;
inline constexpr auto fixedSeedState       = u64{0x853c49e6748fea9bULL};
inline constexpr auto fixedSeedStream      = u64{0xda3e39cb94b95bdbULL};

inline constexpr auto dayInMonth  = 30;
inline constexpr auto monthInYear = 12;
inline constexpr auto epsilon     = 1e-9;
}  // namespace abm::global_setting

namespace abm::global_setting::agent_count {
inline constexpr auto goodsFirm   = 2;
inline constexpr auto capitalFirm = 10;
inline constexpr auto hhold       = 100;
}  // namespace abm::global_setting::agent_count

namespace abm::labor::demander::setting {
inline constexpr RandomParameter lastApplicants     = UniformParameter{.min = 10, .limit = 20};
inline constexpr RandomParameter lastEmployPlan     = UniformParameter{.min = 10, .limit = 20};
inline constexpr RandomParameter offerRate          = UniformParameter{.min = 0.0, .limit = 1.0};
inline constexpr RandomParameter offerRateAdjustVol = UniformParameter{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter lastWage           = UniformParameter{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter wageAdjustVol = UniformParameter{.min = 0.001, .limit = 0.005};
}  // namespace abm::labor::demander::setting

namespace abm::labor::supplier::setting {
inline constexpr RandomParameter productPower =
    NormalParameter{.mean = 1.0, .dev = 1.0 / 3.0, .min = 0.01, .max = 2.0};
inline constexpr auto            jobSampleCnt         = 2;
inline constexpr auto            jobEntryCnt          = 2;
inline constexpr RandomParameter changingJobThreshold = UniformParameter{.min = 0.01, .limit = 0.1};
}  // namespace abm::labor::supplier::setting

namespace abm::base_goods::supplier::setting {
inline constexpr RandomParameter lastDemand     = UniformParameter{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter demandForecast = UniformParameter{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter demandForecastAdjustVol =
    UniformParameter{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter lastSupply      = UniformParameter{.min = 10.0, .limit = 20.0};
inline constexpr RandomParameter lastSalesAmount = UniformParameter{.min = 10.0, .limit = 20.0};
inline constexpr RandomParameter desiredEmploy   = UniformParameter{.min = 10, .limit = 20};
inline constexpr RandomParameter lastMarkup      = UniformParameter{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter markupAdjustVol = UniformParameter{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter priceAdjustVol  = UniformParameter{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter targetInvRatio  = UniformParameter{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter productPower    = UniformParameter{.min = 0.01, .limit = 2.0};
inline constexpr RandomParameter capitalDistributionRate =
    UniformParameter{.min = 0.1, .limit = 0.9};
inline constexpr RandomParameter capitalDepreciationRate =
    UniformParameter{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter inventory = UniformParameter{.min = 10.0, .limit = 100.0};
}  // namespace abm::base_goods::supplier::setting

namespace abm::goods::demander::setting {
inline constexpr auto            goodsSampleCnt = 2;
inline constexpr RandomParameter mpc            = UniformParameter{.min = 0.5, .limit = 0.8};
}  // namespace abm::goods::demander::setting

namespace abm::capital::demander::setting {
inline constexpr RandomParameter lastPurchase    = UniformParameter{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter lastTradeAmount = UniformParameter{.min = 10.0, .limit = 100.0};
inline constexpr auto            goodsSampleCnt  = 2;
}  // namespace abm::capital::demander::setting

namespace abm::government::setting {
inline constexpr auto incomeTaxRate    = 0.1;
inline constexpr auto salesTaxRate     = 0.1;
inline constexpr auto corporateTaxRate = 0.1;
}  // namespace abm::government::setting

namespace abm::finance::setting {
inline constexpr RandomParameter firmInitialAsset = UniformParameter{.min = 100.0, .limit = 1000.0};
inline constexpr RandomParameter hholdInitialAsset = UniformParameter{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter cashRatio         = UniformParameter{.min = 0.1, .limit = 1.0};
}  // namespace abm::finance::setting

namespace abm::config {
struct Config final {
    const int capitalFirm{5};
    const int goodsFirm{2};
    const int hhold{10};
};
};  // namespace abm::config