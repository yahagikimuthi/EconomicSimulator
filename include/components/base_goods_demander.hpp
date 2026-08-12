#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <type_traits>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "world/message.hpp"

namespace abm {
template <Market DemandGoodsType>
class [[nodiscard]] BaseGoodsDemander {
    using Request = std::conditional_t<
        DemandGoodsType == Market::consumerGoods,
        ConsumerGoodsRequest,
        ProductionGoodsRequest>;

  public:
    BaseGoodsDemander(const pcg32 rng, const double mpc) : rng_{rng}, mpc_{mpc} {}

    void afterTrade() {
        if (not isPosting_) return;
        purchasing_ += myRequest_->entry.price * myRequest_->tradeAmount;
    }

    void endStep() {
        myRequest_.reset();
        isPosting_  = false;
        purchasing_ = Money{0.0};
    }

    auto purchase() const -> Money POST(money : money >= Money{0.0}) { return purchasing_; }

  protected:
    auto calcBudget(const Money asset) const -> Money { return asset * mpc_; }

    mutable pcg32                 rng_;
    std::optional<const Request&> myRequest_{std::nullopt};
    bool                          isPosting_{false};
    Money                         purchasing_{0.0};
    const double                  mpc_;
};
}  // namespace abm