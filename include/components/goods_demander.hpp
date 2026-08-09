#pragma once

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "world/message.hpp"

namespace goods::demander {
class [[nodiscard]] GoodsDemander {
  public:
    GoodsDemander(const pcg32 rng, const double mpc, const Step myPhase)
        : rng_{rng}, mpc_{mpc}, myPhase_{myPhase} {}

    void request(
        const Money asset, const Step step, tbb::concurrent_vector<world::GoodsEntry>& entryBox
    );
    void afterTrade() {
        if (not isPosting_) return;
        purchasing_ += myRequest_->entry.price * myRequest_->tradeAmount;
    }
    void endStep() {
        myRequest_  = nullptr;
        isPosting_  = false;
        purchasing_ = Money{0.0};
    }
    auto purchase() const -> Money POST(money : money >= Money{0.0}) { return purchasing_; }

  private:
    auto isPass(
        const Money                                      asset,
        const Step                                       step,
        const tbb::concurrent_vector<world::GoodsEntry>& entryBox
    ) const -> bool;
    auto calcBudget(const Money asset) const -> Money { return asset * mpc_; }

    pcg32                              rng_;
    SafePtr<const world::GoodsRequest> myRequest_{nullptr};
    bool                               isPosting_{false};
    Money                              purchasing_{0.0};
    const double                       mpc_;
    const Step                         myPhase_;
};
}  // namespace goods::demander