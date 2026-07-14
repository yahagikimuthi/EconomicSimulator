#pragma once

#include <cstdint>

namespace config::setting {
constexpr bool          useRuntimeRandomSeed{false};
constexpr std::uint64_t fixedSeedState{0x853c49e6748fea9bULL};
constexpr std::uint64_t fixedSeedStream{0xda3e39cb94b95bdbULL};
}  // namespace config::setting

namespace config::labor_demander {
constexpr double epsilonWage{1e-16};
}

namespace config::labor_supplier {
constexpr int jobSampleCnt{5};
constexpr int jobEntryCnt{3};
}  // namespace config::labor_supplier

namespace config::goods_supplier {
constexpr double epsilonMarkup{1e-12};
constexpr double epsilonPrice{1e-12};
}  // namespace config::goods_supplier

namespace config::goods_demander {
constexpr int maxPurchaseFrequency{7};
constexpr int goodsSampleCnt{5};
}  // namespace config::goods_demander

namespace config::agent_count {
constexpr int firm{100};
constexpr int hhold{2000};
}  // namespace config::agent_count

namespace config {
static thread_local inline int currentStep{0};  // NOLINT
}  // namespace config