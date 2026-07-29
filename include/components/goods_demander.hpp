#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace goods_demander {
class [[nodiscard]] GoodsDemander {
  public:
    GoodsDemander(pcg32& masterRng);

    void request(
        const double asset, const int step, tbb::concurrent_vector<world::GoodsEntry>& entryBox
    );
    void afterTrade() {
        if (not isPosting_) return;
        purchasing_ += myRequest_->tradeAmount * myRequest_->entry.price;
    }
    void endStep() {
        myRequest_  = nullptr;
        isPosting_  = false;
        purchasing_ = 0.0;
    }
    auto purchase() const -> double { return purchasing_; }

  private:
    auto isPass(
        const double                                     asset,
        const int                                        step,
        const tbb::concurrent_vector<world::GoodsEntry>& entryBox
    ) const -> bool;
    auto calcBudget(const double asset) const -> double { return asset * mpc_; }

    pcg32                              rng_;
    SafePtr<const world::GoodsRequest> myRequest_{nullptr};
    bool                               isPosting_{false};
    double                             purchasing_{};
    const double                       mpc_;
    const int                          myPhase_;
    static inline int                  instanceCnt_{};
};
}  // namespace goods_demander