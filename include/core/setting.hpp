#pragma once

#include <concepts>
#include <cstdint>
#include <limits>
#include <string_view>
#include <variant>

namespace abm::setting {
using namespace std::string_view_literals;
}

namespace abm {
template <typename T>
    requires std::same_as<T, double> or std::same_as<T, int>
struct UniformParameter final {
    const T min;
    const T limit;
};
struct NormalParameter final {
    const double mean{0.0};
    const double dev{1.0};
    const double min{-std::numeric_limits<double>::infinity()};
    const double max{std::numeric_limits<double>::infinity()};
};

using RandomParameter =
    std::variant<UniformParameter<int>, UniformParameter<double>, NormalParameter>;
}  // namespace abm

namespace abm::setting {
inline constexpr auto useRuntimeRandomSeed       = false;
inline constexpr auto fixedSeedState             = std::uint64_t{0x853c49e6748fea9bULL};
inline constexpr auto fixedSeedStream            = std::uint64_t{0xda3e39cb94b95bdbULL};
inline constexpr auto simulationResultOutputPath = "../outputs/result.h5"sv;
inline constexpr auto metricDataOutputPath       = "../outputs/metrics.h5"sv;
}  // namespace abm::setting

namespace abm::labor::demander::setting {
inline constexpr RandomParameter lastApplicants = UniformParameter<int>{.min = 10, .limit = 20};
inline constexpr RandomParameter lastEmployPlan = UniformParameter<int>{.min = 10, .limit = 20};
inline constexpr RandomParameter offerRate = UniformParameter<double>{.min = 0.0, .limit = 1.0};
inline constexpr RandomParameter offerRateAdjustVol =
    UniformParameter<double>{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter lastWage = UniformParameter<double>{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter wageAdjustVol =
    UniformParameter<double>{.min = 0.001, .limit = 0.005};
}  // namespace abm::labor::demander::setting

namespace abm::labor::supplier::setting {
inline constexpr RandomParameter productPower =
    NormalParameter{.mean = 1.0, .dev = 1.0 / 3.0, .min = 0.01, .max = 2.0};
inline constexpr auto            jobSampleCnt = 2;
inline constexpr auto            jobEntryCnt  = 2;
inline constexpr RandomParameter changingJobThreshold =
    UniformParameter<double>{.min = 0.01, .limit = 0.1};
}  // namespace abm::labor::supplier::setting

namespace abm::base_goods::supplier::setting {
inline constexpr RandomParameter lastDemand = UniformParameter<double>{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter demandForecast =
    UniformParameter<double>{.min = 10.0, .limit = 100.0};
inline constexpr RandomParameter demandForecastAdjustVol =
    UniformParameter<double>{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter lastSupply = UniformParameter<double>{.min = 10.0, .limit = 20.0};
inline constexpr RandomParameter lastSalesAmount =
    UniformParameter<double>{.min = 10.0, .limit = 20.0};
inline constexpr RandomParameter desiredEmploy = UniformParameter<int>{.min = 10, .limit = 20};
inline constexpr RandomParameter lastMarkup    = UniformParameter<double>{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter markupAdjustVol =
    UniformParameter<double>{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter priceAdjustVol =
    UniformParameter<double>{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter targetInvRatio =
    UniformParameter<double>{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter productPower = UniformParameter<double>{.min = 0.01, .limit = 2.0};
inline constexpr RandomParameter producerGoodsEfficiency =
    UniformParameter<double>{.min = 1.0, .limit = 10.0};
inline constexpr RandomParameter producerGoodsDepreciationRate =
    UniformParameter<double>{.min = 0.1, .limit = 0.2};
inline constexpr RandomParameter inventory = UniformParameter<double>{.min = 10.0, .limit = 100.0};
}  // namespace abm::base_goods::supplier::setting

namespace abm::consumer_goods::demander::setting {
inline constexpr auto            maxPurchaseFrequency = 1;
inline constexpr auto            goodsSampleCnt       = 2;
inline constexpr RandomParameter mpc = UniformParameter<double>{.min = 0.5, .limit = 0.8};
}  // namespace abm::consumer_goods::demander::setting

namespace abm::production_goods::demander::setting {
inline constexpr auto goodsSampleCnt = 2;
}  // namespace abm::production_goods::demander::setting

namespace abm::deposit::supplier::setting {
inline constexpr auto maxSampleCnt = 2;
}

namespace abm::government::setting {
inline constexpr auto incomeTaxRate    = 0.1;
inline constexpr auto salesTaxRate     = 0.1;
inline constexpr auto corporateTaxRate = 0.1;
}  // namespace abm::government::setting

namespace abm::setting::agent_count {
inline constexpr auto bToCFirm = 2;
inline constexpr auto bToBFirm = 5;
inline constexpr auto hhold    = 300;
}  // namespace abm::setting::agent_count

namespace abm::setting::agent_finance {
inline constexpr RandomParameter firm  = UniformParameter<double>{.min = 100.0, .limit = 1000.0};
inline constexpr RandomParameter hhold = UniformParameter<double>{.min = 10.0, .limit = 100.0};
}  // namespace abm::setting::agent_finance

namespace abm::setting::save_name {
inline constexpr auto firmAssets        = "firmAssets"sv;
inline constexpr auto postedEmployments = "postedEmployments"sv;
inline constexpr auto postedWages       = "postedWages"sv;
inline constexpr auto employments       = "employments"sv;
inline constexpr auto sumWages          = "sumWages"sv;
inline constexpr auto prices            = "prices"sv;
inline constexpr auto supplies          = "supplies"sv;
inline constexpr auto markups           = "markups"sv;
inline constexpr auto inventories       = "inventories"sv;
inline constexpr auto householdAssets   = "householdAssets"sv;
inline constexpr auto wages             = "wages"sv;
}  // namespace abm::setting::save_name