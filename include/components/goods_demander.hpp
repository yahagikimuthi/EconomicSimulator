#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>
#include <utility>

#include "core/base.hpp"
#include "core/forward.hpp"

namespace goods_demander {
struct Posting {
    using GoodsMarketCoordinate =
        std::pair<SafePtr<const world::GoodsEntry>, SafePtr<const world::GoodsRequest>>;
    GoodsMarketCoordinate myRequest_{nullptr, nullptr};
    bool                  isPosting_{false};
};
struct Purchasing {
    double purchase_{};
};
struct Parameter {
    const double mpc_;
    const int    myPhase_;
};
struct Component {
    pcg32      rng_;
    Posting    posting_;
    Purchasing purchasing_;
    Parameter  parameter_;

    Component(const std::uint64_t state, const std::uint64_t stream);
    [[nodiscard]] auto purchase() const -> double { return purchasing_.purchase_; }

  private:
    static inline int instanceCnt_{};
};
}  // namespace goods_demander