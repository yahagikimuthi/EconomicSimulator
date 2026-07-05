#pragma once

namespace config::init_setup::setting {
constexpr unsigned int seed{42};
}  // namespace config::init_setup::setting

namespace config::setting {
constexpr int seed{42};
}
namespace config::labor_demander {
constexpr double epsilonWage{1e-16};
}
namespace config::goods_supplier {
constexpr double epsilonMarkup{0.001};
constexpr double epsilonPrice{1e-12};
}  // namespace config::goods_supplier