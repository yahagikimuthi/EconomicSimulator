#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <type_traits>

#include "core/values/common.hpp"
#include "util.hpp"
#include "world/message.hpp"

namespace abm::base_goods::demander {
class BudgetCalculator {
  public:
    BudgetCalculator(const double mpc) : mpc_{mpc} {}

    auto calcBudget(const Money asset) const -> Money { return asset * mpc_; }

  private:
    const double mpc_;
};
}  // namespace abm::base_goods::demander

namespace abm {
template <Market DemandGoodsType>
    requires(DemandGoodsType != Market::labor)
class [[nodiscard]] BaseGoodsDemander {
    using Request = std::conditional_t<
        DemandGoodsType == Market::consumerGoods,
        ConsumerGoodsRequest,
        ProductionGoodsRequest>;

  protected:
    BaseGoodsDemander(
        const RandomGenerator rng, const base_goods::demander::BudgetCalculator budgetCalculator
    )
        : rng_{rng}, budgetCalculator_{budgetCalculator} {}

    mutable RandomGenerator                rng_;
    std::optional<const Request&>          myRequest_{std::nullopt};
    bool                                   isPosting_{false};
    base_goods::demander::BudgetCalculator budgetCalculator_;
};
}  // namespace abm