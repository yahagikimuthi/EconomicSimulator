#pragma once

namespace config::setting {
constexpr unsigned int seed{42};
}  // namespace config::setting

namespace config::labor_demander {
constexpr double epsilonWage{1e-16};
}

namespace config::labor_supplier {
constexpr int jobSampleCnt{5};
constexpr int jobEntryCnt{3};
}  // namespace config::labor_supplier

namespace config::goods_supplier {
constexpr double epsilonMarkup{0.001};
constexpr double epsilonPrice{1e-12};
}  // namespace config::goods_supplier

namespace config::goods_demander {
constexpr int maxPurchaseFrequency{7};
constexpr int goodsSampleCnt{5};
}  // namespace config::goods_demander

namespace config::agent_count {
constexpr int firm{10};
constexpr int hhold{100};
}  // namespace config::agent_count

namespace config {
static inline int currentStep{0};  // NOLINT
}  // namespace config