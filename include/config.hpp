#pragma once

#include <cstdint>
#include <string_view>

namespace abm::config::setting {
constexpr bool             useRuntimeRandomSeed{false};
constexpr std::uint64_t    fixedSeedState{0x853c49e6748fea9bULL};
constexpr std::uint64_t    fixedSeedStream{0xda3e39cb94b95bdbULL};
constexpr std::string_view simulationResultOutputPath{"../outputs/result.h5"};
constexpr std::string_view metricDataOutputPath{"../outputs/metrics.h5"};
}  // namespace abm::config::setting

namespace abm::config::labor_supplier {
constexpr int jobSampleCnt{2};
constexpr int jobEntryCnt{2};
}  // namespace abm::config::labor_supplier

namespace abm::config::goods_supplier {
constexpr double epsilonMarkup{1e-12};
constexpr double epsilonPrice{1e-12};
}  // namespace abm::config::goods_supplier

namespace abm::config::goods_demander {
constexpr int maxPurchaseFrequency{7};
constexpr int goodsSampleCnt{2};
}  // namespace abm::config::goods_demander

namespace abm::config::agent_count {
constexpr int BtoCFirm{2};
constexpr int BtoBFirm{2};
constexpr int hhold{1000};
}  // namespace abm::config::agent_count

namespace abm::config::save_name {
constexpr std::string_view firmAssets{"firmAssets"};
constexpr std::string_view postedEmployments{"postedEmployments"};
constexpr std::string_view employments{"employments"};
constexpr std::string_view prices{"prices"};
constexpr std::string_view supplies{"supplies"};
constexpr std::string_view markups{"markups"};
constexpr std::string_view inventories{"inventories"};
constexpr std::string_view householdAssets{"householdAssets"};
constexpr std::string_view wages{"wages"};
}  // namespace abm::config::save_name