#pragma once

#include <cstdint>
#include <string>

namespace config::setting {
constexpr bool             useRuntimeRandomSeed{false};
constexpr std::uint64_t    fixedSeedState{0x853c49e6748fea9bULL};
constexpr std::uint64_t    fixedSeedStream{0xda3e39cb94b95bdbULL};
constexpr std::string_view simulationResultOutputPath{"../outputs/result.h5"};
constexpr std::string_view metricDataOutputPath{"../outputs/metrics.h5"};
}  // namespace config::setting

namespace config::labor_demander {
constexpr double epsilonWage{1e-16};
}

namespace config::labor_supplier {
constexpr int jobSampleCnt{2};
constexpr int jobEntryCnt{2};
}  // namespace config::labor_supplier

namespace config::goods_supplier {
constexpr double epsilonMarkup{1e-12};
constexpr double epsilonPrice{1e-12};
}  // namespace config::goods_supplier

namespace config::goods_demander {
constexpr int maxPurchaseFrequency{7};
constexpr int goodsSampleCnt{2};
}  // namespace config::goods_demander

namespace config::agent_count {
constexpr int firm{20};
constexpr int hhold{400};
}  // namespace config::agent_count

namespace config::save_name {
constexpr std::string_view firmAssets{"firmAssets"};
constexpr std::string_view postedEmployments{"postedEmployments"};
constexpr std::string_view employments{"employments"};
constexpr std::string_view prices{"prices"};
constexpr std::string_view supplies{"supplies"};
constexpr std::string_view markups{"markups"};
constexpr std::string_view inventories{"inventories"};
constexpr std::string_view householdAssets{"householdAssets"};
constexpr std::string_view wages{"wages"};
}  // namespace config::save_name