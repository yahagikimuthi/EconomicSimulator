#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <type_traits>

#include "core/values/common.hpp"
#include "world/message.hpp"

namespace abm {
template <Market DemandGoodsType>
class [[nodiscard]] BaseGoodsDemander {
    using Request = std::conditional_t<
        DemandGoodsType == Market::consumerGoods,
        ConsumerGoodsRequest,
        ProductionGoodsRequest>;

  protected:
    BaseGoodsDemander(const pcg32 rng, const double mpc) : rng_{rng}, mpc_{mpc} {}

    auto calcBudget(const Money asset) const -> Money { return asset * mpc_; }

    mutable pcg32                 rng_;
    std::optional<const Request&> myRequest_{std::nullopt};
    bool                          isPosting_{false};
    const double                  mpc_;
};
}  // namespace abm