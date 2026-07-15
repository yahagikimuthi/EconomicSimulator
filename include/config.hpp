#pragma once

#include <cstdint>
#include <string>

namespace config::setting {
constexpr bool          useRuntimeRandomSeed{false};
constexpr std::uint64_t fixedSeedState{0x853c49e6748fea9bULL};
constexpr std::uint64_t fixedSeedStream{0xda3e39cb94b95bdbULL};
const std::string       simulationResultOutputPath{"../outputs/result.h5"};
const std::string       metricDataOutputPath{"../outputs/metrics.h5"};
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

namespace config {
static thread_local inline int currentStep{0};  // NOLINT
}  // namespace config

namespace config::save_name {
static inline const std::string firmAssets{"firmAssets"};
static inline const std::string postedEmployments{"postedEmployments"};
static inline const std::string employments{"employments"};
static inline const std::string prices{"prices"};
static inline const std::string supplies{"supplies"};
static inline const std::string markups{"markups"};
static inline const std::string inventories{"inventories"};
static inline const std::string householdAssets{"householdAssets"};
static inline const std::string wages{"wages"};
}  // namespace config::save_name