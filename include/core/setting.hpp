#pragma once

#include <cstdint>
#include <limits>
#include <string_view>
#include <variant>

namespace abm::setting {
using namespace std::string_view_literals;
}

namespace abm {
template <typename T>
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
constexpr auto useRuntimeRandomSeed       = false;
constexpr auto fixedSeedState             = std::uint64_t{0x853c49e6748fea9bULL};
constexpr auto fixedSeedStream            = std::uint64_t{0xda3e39cb94b95bdbULL};
constexpr auto simulationResultOutputPath = "../outputs/result.h5"sv;
constexpr auto metricDataOutputPath       = "../outputs/metrics.h5"sv;
}  // namespace abm::setting

namespace abm::labor::demander::setting {
constexpr RandomParameter lastApplicants     = UniformParameter<int>{.min = 10, .limit = 20};
constexpr RandomParameter lastEmployPlan     = UniformParameter<int>{.min = 10, .limit = 20};
constexpr RandomParameter offerRate          = UniformParameter<double>{.min = 0.0, .limit = 1.0};
constexpr RandomParameter offerRateAdjustVol = UniformParameter<double>{.min = 0.1, .limit = 0.2};
constexpr RandomParameter lastWage      = UniformParameter<double>{.min = 10.0, .limit = 100.0};
constexpr RandomParameter wageAdjustVol = UniformParameter<double>{.min = 0.001, .limit = 0.005};
}  // namespace abm::labor::demander::setting

namespace abm::labor::supplier::setting {
constexpr RandomParameter productPower =
    NormalParameter{.mean = 1.0, .dev = 1.0 / 3.0, .min = 0.01, .max = 2.0};
constexpr auto            jobSampleCnt = 2;
constexpr auto            jobEntryCnt  = 2;
constexpr RandomParameter changingJobThreshold =
    UniformParameter<double>{.min = 0.01, .limit = 0.1};
}  // namespace abm::labor::supplier::setting

namespace abm::base_goods::supplier::setting {
constexpr RandomParameter lastDemand     = UniformParameter<double>{.min = 10.0, .limit = 100.0};
constexpr RandomParameter demandForecast = UniformParameter<double>{.min = 10.0, .limit = 100.0};
constexpr RandomParameter demandForecastAdjustVol =
    UniformParameter<double>{.min = 0.1, .limit = 0.2};
constexpr RandomParameter lastSupply      = UniformParameter<double>{.min = 10.0, .limit = 20.0};
constexpr RandomParameter lastSalesAmount = UniformParameter<double>{.min = 10.0, .limit = 20.0};
constexpr RandomParameter desiredEmploy   = UniformParameter<int>{.min = 10, .limit = 20};
constexpr RandomParameter lastMarkup      = UniformParameter<double>{.min = 0.1, .limit = 0.2};
constexpr RandomParameter markupAdjustVol = UniformParameter<double>{.min = 0.1, .limit = 0.2};
constexpr RandomParameter priceAdjustVol  = UniformParameter<double>{.min = 0.1, .limit = 0.2};
constexpr RandomParameter targetInvRatio  = UniformParameter<double>{.min = 0.1, .limit = 0.2};
constexpr RandomParameter productPower    = UniformParameter<double>{.min = 0.01, .limit = 2.0};
constexpr RandomParameter producerGoodsEfficiency =
    UniformParameter<double>{.min = 1.0, .limit = 10.0};
constexpr RandomParameter inventory = UniformParameter<double>{.min = 10.0, .limit = 100.0};
}  // namespace abm::base_goods::supplier::setting

namespace abm::consumer_goods::demander::setting {
constexpr auto            maxPurchaseFrequency = 1;
constexpr auto            goodsSampleCnt       = 2;
constexpr RandomParameter mpc                  = UniformParameter<double>{.min = 0.5, .limit = 0.8};
}  // namespace abm::consumer_goods::demander::setting

namespace abm::production_goods::demander::setting {
constexpr auto            goodsSampleCnt = 2;
constexpr RandomParameter mpc            = UniformParameter<double>{.min = 0.5, .limit = 0.8};
}  // namespace abm::production_goods::demander::setting

namespace abm::setting::agent_count {
constexpr auto bToCFirm = 10;
constexpr auto bToBFirm = 10;
constexpr auto hhold    = 100;
}  // namespace abm::setting::agent_count

namespace abm::setting::agent_finance {
constexpr RandomParameter firm  = UniformParameter<double>{.min = 100.0, .limit = 1000.0};
constexpr RandomParameter hhold = UniformParameter<double>{.min = 10.0, .limit = 100.0};
}  // namespace abm::setting::agent_finance

namespace abm::setting::save_name {
constexpr auto firmAssets        = "firmAssets"sv;
constexpr auto postedEmployments = "postedEmployments"sv;
constexpr auto postedWages       = "postedWages"sv;
constexpr auto employments       = "employments"sv;
constexpr auto sumWages          = "sumWages"sv;
constexpr auto prices            = "prices"sv;
constexpr auto supplies          = "supplies"sv;
constexpr auto markups           = "markups"sv;
constexpr auto inventories       = "inventories"sv;
constexpr auto householdAssets   = "householdAssets"sv;
constexpr auto wages             = "wages"sv;
}  // namespace abm::setting::save_name