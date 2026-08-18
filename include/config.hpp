#pragma once

#include <cstdint>
#include <string_view>

namespace abm::config {
using namespace std::string_view_literals;
}

namespace abm::config::setting {
constexpr auto useRuntimeRandomSeed       = false;
constexpr auto fixedSeedState             = std::uint64_t{0x853c49e6748fea9bULL};
constexpr auto fixedSeedStream            = std::uint64_t{0xda3e39cb94b95bdbULL};
constexpr auto simulationResultOutputPath = "../outputs/result.h5"sv;
constexpr auto metricDataOutputPath       = "../outputs/metrics.h5"sv;
}  // namespace abm::config::setting

namespace abm::config::labor_supplier {
constexpr auto jobSampleCnt = 2;
constexpr auto jobEntryCnt  = 2;
}  // namespace abm::config::labor_supplier

namespace abm::config::goods_supplier {
constexpr auto epsilonMarkup = 1e-12;
constexpr auto epsilonPrice  = 1e-12;
}  // namespace abm::config::goods_supplier

namespace abm::config::goods_demander {
constexpr auto maxPurchaseFrequency = 7;
constexpr auto goodsSampleCnt       = 2;
}  // namespace abm::config::goods_demander

namespace abm::config::agent_count {
constexpr auto BtoCFirm = 2;
constexpr auto BtoBFirm = 2;
constexpr auto hhold    = 1000;
}  // namespace abm::config::agent_count

namespace abm::config::save_name {
constexpr auto firmAssets        = "firmAssets"sv;
constexpr auto postedEmployments = "postedEmployments"sv;
constexpr auto employments       = "employments"sv;
constexpr auto prices            = "prices"sv;
constexpr auto supplies          = "supplies"sv;
constexpr auto markups           = "markups"sv;
constexpr auto inventories       = "inventories"sv;
constexpr auto householdAssets   = "householdAssets"sv;
constexpr auto wages             = "wages"sv;
}  // namespace abm::config::save_name